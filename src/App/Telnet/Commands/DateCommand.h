#ifndef DATE_COMMAND_CLASS
#define DATE_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class DateCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "date"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        const char *tPtr = tArguments;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr == '\0') return HandleSystemDateTime(tClient);
        if (strcmp(tPtr, "rtc") == 0) return HandleRTCDateTime(tClient);
        else if (strncmp(tPtr, "rtc set ", 8) == 0) return HandleRTCSet(tPtr + 8, tClient);
        else if (strcmp(tPtr, "rtc sync-from-ntp") == 0) return HandleRTCSyncFromNTP(tClient);
        else if (strcmp(tPtr, "rtc sync-to-system") == 0) return HandleRTCSyncToSystem(tClient);
        else if (strncmp(tPtr, "rtc", 3) == 0) {
          tClient.print(F(COLOR_RED "\r\n  Unknown rtc subcommand!" COLOR_YELLOW "\r\n  Usage: date rtc | date rtc set | date rtc sync-from-ntp | date rtc sync-to-system\r\n\r\n" COLOR_WHITE));
          return false;
        }
        return HandleSystemDateTime(tClient);
      }
      const char *Help() const override {
        return "date                             " COLOR_YELLOW "- show system date and time\r\n  " COLOR_WHITE
               "date rtc                         " COLOR_YELLOW "- show RTC date and time\r\n  " COLOR_WHITE
               "date rtc set YYYY.MM.DD HH:MM:SS " COLOR_YELLOW "- set RTC date and time\r\n  " COLOR_WHITE
               "date rtc sync-from-ntp           " COLOR_YELLOW "- sync RTC from NTP\r\n  " COLOR_WHITE
               "date rtc sync-to-system          " COLOR_YELLOW "- sync system time from RTC" COLOR_WHITE;
      }
    private:
      static time_t ToLocalTime(time_t tUtc) {
        const SNTPConfig tCfg = CFG.Get<SNTPConfig>();
        return (time_t)((unsigned long)tUtc + (unsigned long)tCfg.GMTOffset);
      }

      bool HandleRTCDateTime(WiFiClient &tClient) {
        RTC.Init(false);
        if (!RTC.IsAvailable()) {
          tClient.print(F(COLOR_RED "\r\n  No RTC detected\r\n\r\n" COLOR_WHITE));
          return false;
        }
        tClient.print(F(COLOR_GREEN "\r\n  RTC DateTime (local):\r\n" COLOR_WHITE));
        unsigned long tEpoch = RTC.GetEpoch();
        if (tEpoch == 0) {
          tClient.print(F(COLOR_RED "\r\n  Error: RTC read failed\r\n\r\n" COLOR_WHITE));
          return false;
        }
        time_t tNowUtc = (time_t)tEpoch;
        time_t tNowLocal = ToLocalTime(tNowUtc);
        struct tm tTimeInfo;
        gmtime_r(&tNowLocal, &tTimeInfo);
        char tBuffer[16];
        strftime(tBuffer, sizeof(tBuffer), "%Y.%m.%d", &tTimeInfo);
        tClient.print(F("\r\n  Date: "));
        tClient.print(tBuffer);
        strftime(tBuffer, sizeof(tBuffer), "%H:%M:%S", &tTimeInfo);
        tClient.print(F("\r\n  Time: "));
        tClient.print(tBuffer);
        tClient.print(F("\r\n\r\n"));
        return true;
      }
      bool HandleRTCSet(const char *tArgs, WiFiClient &tClient) {
        RTC.Init(false);
        if (!RTC.IsAvailable()) {
          tClient.print(F(COLOR_RED "\r\n\r\n  No RTC detected\r\n\r\n" COLOR_WHITE));
          return false;
        }
        SRTCDateTime tDateTime;
        int tYear, tMonth, tDay, tHour, tMin, tSec;
        if (sscanf(tArgs, "%d.%d.%d %d:%d:%d", &tYear, &tMonth, &tDay, &tHour, &tMin, &tSec) != 6) {
          tClient.print(F(COLOR_RED "\r\n\r\n  Invalid format!" COLOR_YELLOW "\r\n  Usage: date rtc set YYYY.MM.DD HH:MM:SS\r\n\r\n" COLOR_WHITE));
          return false;
        }
        if (tYear < 2026 || tYear > 2099) { 
          tClient.print(F(COLOR_RED "\r\n\r\n  Invalid year! Must be 2026-2099\r\n\r\n" COLOR_WHITE));
          return false;
        }
        if (tMonth < 1 || tMonth > 12) {
          tClient.print(F(COLOR_RED "\r\n\r\n  Invalid month! Must be 1-12\r\n\r\n" COLOR_WHITE));
          return false;
        }
        if (tDay < 1 || tDay > 31) {
          tClient.print(F(COLOR_RED "\r\n  Invalid day! Must be 1-31\r\n\r\n" COLOR_WHITE));
          return false;
        }
        if (tHour < 0 || tHour > 23) {
          tClient.print(F(COLOR_RED "\r\n\r\n  Invalid hour! Must be 0-23\r\n\r\n" COLOR_WHITE));
          return false;
        }
        if (tMin < 0 || tMin > 59) {
          tClient.print(F(COLOR_RED "\r\n\r\n  Invalid minute! Must be 0-59\r\n\r\n" COLOR_WHITE));
          return false;
        }
        if (tSec < 0 || tSec > 59) {
          tClient.print(F(COLOR_RED "\r\n\r\n  Invalid second! Must be 0-59\r\n\r\n" COLOR_WHITE));
          return false;
        }
        tDateTime.Year = tYear;
        tDateTime.Month = tMonth;
        tDateTime.Day = tDay;
        tDateTime.Hour = tHour;
        tDateTime.Minute = tMin;
        tDateTime.Second = tSec;
        tClient.print(F("\r\n\r\n  Setting RTC datetime...\r\n"));
        // Treat user input as LOCAL time, store RTC as UTC internally.
        unsigned long tLocalEpoch = 0;
        {
          // Same algorithm as RTC_::DateTimeToEpoch (duplicated here to avoid changing RTC_ API)
          unsigned long tDays = 0;
          for (uint16_t y = 1970; y < tDateTime.Year; y++) {
            tDays += (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
          }
          static const uint8_t tDaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
          for (uint8_t m = 1; m < tDateTime.Month; m++) {
            tDays += tDaysInMonth[m - 1];
            if (m == 2 && (tDateTime.Year % 4 == 0 && (tDateTime.Year % 100 != 0 || tDateTime.Year % 400 == 0))) tDays++;
          }
          tDays += tDateTime.Day - 1;
          tLocalEpoch = tDays * 86400UL + tDateTime.Hour * 3600UL + tDateTime.Minute * 60UL + tDateTime.Second;
        }
        const SNTPConfig tCfg = CFG.Get<SNTPConfig>();
        unsigned long tUtcEpoch = (tLocalEpoch >= (unsigned long)tCfg.GMTOffset) ? (tLocalEpoch - (unsigned long)tCfg.GMTOffset) : 0UL;
        if (tUtcEpoch == 0) {
          tClient.print(F(COLOR_RED "  Failed to convert local time to UTC\r\n\r\n" COLOR_WHITE));
          return false;
        }
        if (RTC.SetFromEpoch(tUtcEpoch)) {
          char tBuffer[16];
          tClient.print(F("\r\n  RTC datetime set!\r\n"));
          tClient.print(F(COLOR_GREEN "\r\n  New RTC DateTime (local): \r\n" COLOR_WHITE));
          HandleRTCDateTime(tClient);
          tClient.print(F("\r\n\r\n  Syncing system time from RTC...\r\n"));
          if (RTC.SyncToSystem()) {
            tClient.print(F("\r\n  System time synced!\r\n"));
            HandleSystemDateTime(tClient);
          } else tClient.print(F(COLOR_YELLOW "  System time sync failed!\r\n\r\n" COLOR_WHITE));
          return true;
        } else {
          tClient.print(F(COLOR_RED "  Failed to set RTC datetime\r\n\r\n" COLOR_WHITE));
          return false;
        }
      }
      bool HandleRTCSyncFromNTP(WiFiClient &tClient) {
        RTC.Init(false);
        if (!RTC.IsAvailable()) {
          tClient.print(F(COLOR_RED "\r\n  No RTC detected\r\n\r\n" COLOR_WHITE));
          return false;
        }
        if (CFG.Get<SConnectionConfig>().ApModeEnable) {
          tClient.print(F(COLOR_RED "\r\n  NTP not available in AP mode!\r\n\r\n" COLOR_WHITE));
          return false;
        }
        tClient.print(F("\r\n  Connecting to NTP server...\r\n"));
        NTP.Init();
        if (!NTP.SyncSystemTime()) {
          tClient.print(F(COLOR_RED "  NTP sync failed!\r\n\r\n" COLOR_WHITE));
          NTP.End();
          return false;
        }
        tClient.print(F("  Syncing RTC from NTP...\r\n"));
        if (RTC.SyncFromNTP()) {
          char tBuffer[16];
          tClient.print(F("\r\n  RTC synced!\r\n"));
          tClient.print(F(COLOR_GREEN "\r\n  New RTC DateTime: \r\n" COLOR_WHITE));
          RTC.GetDate(tBuffer, sizeof(tBuffer));
          tClient.print(F("\r\n  New Date: "));
          tClient.print(tBuffer);
          RTC.GetTime(tBuffer, sizeof(tBuffer));
          tClient.print(F("\r\n  New Time: "));
          tClient.print(tBuffer);
          tClient.print(F("\r\n\r\n  Syncing system time from RTC...\r\n"));
          if (RTC.SyncToSystem()) {
            tClient.print(F("\r\n  System time synced!\r\n"));
            HandleSystemDateTime(tClient);
          } else tClient.print(F(COLOR_YELLOW "  System time sync failed!\r\n\r\n" COLOR_WHITE));
          NTP.End();
          return true;
        } else {
          tClient.print(F(COLOR_RED "\r\n  Sync failed!\r\n\r\n" COLOR_WHITE));
          NTP.End();
          return false;
        }
      }
      bool HandleRTCSyncToSystem(WiFiClient &tClient) {
        RTC.Init(false);
        if (!RTC.IsAvailable()) {
          tClient.print(F(COLOR_RED "\r\n  No RTC detected\r\n\r\n" COLOR_WHITE));
          return false;
        }
        tClient.print(F("\r\n  Syncing system time from RTC...\r\n\r\n"));
        if (RTC.SyncToSystem()) {
          tClient.print(F(COLOR_GREEN "  System time synced!\r\n\r\n" COLOR_WHITE));
          HandleSystemDateTime(tClient);
          return true;
        } else {
          tClient.print(F(COLOR_RED "  Sync failed!\r\n\r\n" COLOR_WHITE));
          return false;
        }
      }
      bool HandleSystemDateTime(WiFiClient &tClient) {
        time_t tNowUtc;
        time(&tNowUtc);
        time_t tNowLocal = ToLocalTime(tNowUtc);
        struct tm tTimeInfo;
        gmtime_r(&tNowLocal, &tTimeInfo);
        tClient.print(F(COLOR_GREEN "\r\n  System DateTime (local):\r\n" COLOR_WHITE));
        char tBuffer[16];
        strftime(tBuffer, sizeof(tBuffer), "%Y.%m.%d", &tTimeInfo);
        tClient.print(F("\r\n  Date: "));
        tClient.print(tBuffer);
        strftime(tBuffer, sizeof(tBuffer), "%H:%M:%S", &tTimeInfo);
        tClient.print(F("\r\n  Time: "));
        tClient.print(tBuffer);
        tClient.print(F("\r\n\r\n"));
        return true;
      }
  };

}

#endif