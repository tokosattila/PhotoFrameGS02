#ifndef LOG_COMMAND_CLASS
#define LOG_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class LogCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "log";
      }

      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        if (!tArguments || !tArguments[0]) {
          tClient.println(F("log status                        - show logging status"));
          tClient.println(F("log info                          - show supported log levels"));
          tClient.println(F("log flush                         - flush log buffer to file"));
          return true;
        }

        if (strcmp(tArguments, "status") == 0) {
          return HandleStatus(tClient);
        } else if (strcmp(tArguments, "info") == 0) {
          return HandleInfo(tClient);
        } else if (strcmp(tArguments, "flush") == 0) {
          return HandleFlush(tClient);
        }

        tClient.print(F("Unknown log command: "));
        tClient.println(tArguments);
        return false;
      }

      const char *Help() const override {
        return "log                              " COLOR_YELLOW "- logging status & management" COLOR_WHITE;
      }

    private:
      bool HandleStatus(WiFiClient &tClient) {
        tClient.println();
        tClient.println(F("─────────────────────────────────────────────"));
        tClient.println(F("Logging Status:"));

        const SDeviceConfig tDevCfg = CFG.Get<SDeviceConfig>();
        tClient.print(F("  Enabled:        "));
        tClient.println(tDevCfg.LogManagerEnabled ? "YES" : "NO");

        time_t tNow = time(nullptr);
        struct tm tTm = {};
        if (tNow >= 1000000000L) {
          localtime_r(&tNow, &tTm);
          char tDatePath[64] = "";
          snprintf(tDatePath, sizeof(tDatePath), "logs/%04d/%02d/%02d/",
                   tTm.tm_year + 1900, tTm.tm_mon + 1, tTm.tm_mday);
          tClient.print(F("  Log Directory:  "));
          tClient.println(tDatePath);

          char tDateStr[16] = "";
          snprintf(tDateStr, sizeof(tDateStr), "%04d%02d%02d",
                   tTm.tm_year + 1900, tTm.tm_mon + 1, tTm.tm_mday);
          tClient.print(F("  Today's File:   "));
          tClient.println(tDateStr);

          tClient.println(F("  Current Size:   N/A"));
          tClient.println(F("  Roll-over Files: N/A"));
        } else {
          tClient.println(F("  Log Directory:  <no time sync>"));
          tClient.println(F("  Today's File:   <no time sync>"));
        }

        tClient.println(F("  Buffer Usage:   N/A"));
        tClient.println(F("─────────────────────────────────────────────"));
        tClient.println();
        return true;
      }

      bool HandleInfo(WiFiClient &tClient) {
        tClient.println();
        tClient.println(F("─────────────────────────────────────────────"));
        tClient.println(F("Log Levels (Supported):"));
        tClient.println(F("  [0] Boot       [1] Halt       [2] Storage"));
        tClient.println(F("  [3] Wifi       [4] Ntp        [5] Rtc"));
        tClient.println(F("  [6] Battery    [7] Image      [8] Sleep"));
        tClient.println(F("  [9] Ota        [10] Warn      [11] Error"));
        tClient.println(F("─────────────────────────────────────────────"));
        tClient.println();
        return true;
      }

      bool HandleFlush(WiFiClient &tClient) {
        LOG.Flush();
        tClient.println();
        tClient.println(F("─────────────────────────────────────────────"));
        tClient.println(F("Buffer flushed to log file"));
        tClient.println(F("─────────────────────────────────────────────"));
        tClient.println();
        return true;
      }
  };

}

#endif
