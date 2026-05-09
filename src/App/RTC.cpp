#include <App/RTC.h>

namespace App {

  RTC_ &RTC_::Instance() {
    static RTC_ tInstance;
    return tInstance;
  }

  RTC_::RTC_() {
    mMutex = xSemaphoreCreateRecursiveMutex();
  }

  RTC_::~RTC_() {
    if (mMutex) {
      vSemaphoreDelete(mMutex);
      mMutex = nullptr;
    }
  }

  void RTC_::Lock() {
    if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
  }

  void RTC_::Unlock() {
    if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
  }

  bool RTC_::Init(bool tVerbose) {
    Guard tLock;
    mWire.begin(mSdaPin, mSclPin);
    mAvailable = TryI2C();
    if (tVerbose) {
      if (mAvailable) {
        xLOG("RTC found → PCF8563");
        PrintInfo();
      } else xLOG("No RTC detected on I2C bus");
    }
    return mAvailable;
  }

  void RTC_::End() {
    Guard tLock;
    mWire.end();
    mAvailable = false;
  }

  bool RTC_::TryI2C() {
    mWire.beginTransmission(mAddress);
    return (mWire.endTransmission() == 0);
  }

  bool RTC_::IsDateTimePlausible(const SRTCDateTime &tDateTime) {
    if (tDateTime.Year < 2000 || tDateTime.Year > 2099) {
      xLOG("RTC plausibility check FAIL: year %u out of range [2000-2099]", tDateTime.Year);
      return false;
    }
    if (tDateTime.Month < 1 || tDateTime.Month > 12) {
      xLOG("RTC plausibility check FAIL: month %u out of range [1-12]", tDateTime.Month);
      return false;
    }
    if (tDateTime.Day < 1 || tDateTime.Day > 31) {
      xLOG("RTC plausibility check FAIL: day %u out of range [1-31]", tDateTime.Day);
      return false;
    }
    if (tDateTime.Hour > 23) {
      xLOG("RTC plausibility check FAIL: hour %u out of range [0-23]", tDateTime.Hour);
      return false;
    }
    if (tDateTime.Minute > 59) {
      xLOG("RTC plausibility check FAIL: minute %u out of range [0-59]", tDateTime.Minute);
      return false;
    }
    if (tDateTime.Second > 59) {
      xLOG("RTC plausibility check FAIL: second %u out of range [0-59]", tDateTime.Second);
      return false;
    }
    if (tDateTime.DayOfWeek > 6) {
      xLOG("RTC plausibility check FAIL: day of week %u out of range [0-6]", tDateTime.DayOfWeek);
      return false;
    }
    return true;
  }

  bool RTC_::ReadDateTimeRaw(SRTCDateTime &tDateTime, bool &tVLFlagSet) {
    mWire.beginTransmission(mAddress);
    mWire.write(0x02);
    if (mWire.endTransmission() != 0) {
      xLOG("RTC I2C register select failed");
      return false;
    }
    if (mWire.requestFrom(mAddress, (uint8_t)7) < 7) {
      xLOG("RTC I2C read < 7 bytes");
      return false;
    }
    uint8_t tSecRaw = mWire.read();
    tVLFlagSet = (tSecRaw & 0x80) != 0;
    if (tVLFlagSet) {
      xLOG("RTC VL (low voltage) flag set → clock may be invalid");
    }
    tDateTime.Second = BcdToDec(tSecRaw & 0x7F);
    tDateTime.Minute = BcdToDec(mWire.read() & 0x7F);
    tDateTime.Hour = BcdToDec(mWire.read() & 0x3F);
    tDateTime.Day = BcdToDec(mWire.read() & 0x3F);
    tDateTime.DayOfWeek = BcdToDec(mWire.read() & 0x07);
    uint8_t tMonthReg = mWire.read();
    tDateTime.Month = BcdToDec(tMonthReg & 0x1F);
    tDateTime.Year = 2000 + BcdToDec(mWire.read());
    if (tMonthReg & 0x80) tDateTime.Year += 100;
    return true;
  }

  bool RTC_::ReadDateTimeWithRetry(SRTCDateTime &tDateTime) {
    for (uint8_t tRetry = 0; tRetry < I2C_RETRY_COUNT; tRetry++) {
      bool tVLSet = false;
      if (!ReadDateTimeRaw(tDateTime, tVLSet)) {
        if (tRetry < I2C_RETRY_COUNT - 1) {
          xLOG("RTC read attempt %u/%u failed, retrying...", tRetry + 1, I2C_RETRY_COUNT);
          vTaskDelay(pdMS_TO_TICKS(I2C_RETRY_DELAY_MS));
        }
        continue;
      }
      if (!IsDateTimePlausible(tDateTime)) {
        if (tRetry < I2C_RETRY_COUNT - 1) {
          xLOG("RTC data implausible, retrying... (attempt %u/%u)", tRetry + 1, I2C_RETRY_COUNT);
          vTaskDelay(pdMS_TO_TICKS(I2C_RETRY_DELAY_MS));
        }
        continue;
      }
      xLOG("RTC read OK @ attempt %u/%u", tRetry + 1, I2C_RETRY_COUNT);
      return true;
    }
    xLOG("RTC read failed after %u attempts", I2C_RETRY_COUNT);
    return false;
  }

  bool RTC_::WriteDateTimeWithRetry(const SRTCDateTime &tDateTime) {
    if (!IsDateTimePlausible(tDateTime)) {
      xLOG("RTC cannot write implausible date/time");
      return false;
    }
    for (uint8_t tRetry = 0; tRetry < I2C_RETRY_COUNT; tRetry++) {
      mWire.beginTransmission(mAddress);
      mWire.write(0x02);
      mWire.write(DecToBcd(tDateTime.Second) & 0x7F);
      mWire.write(DecToBcd(tDateTime.Minute));
      mWire.write(DecToBcd(tDateTime.Hour));
      mWire.write(DecToBcd(tDateTime.Day));
      mWire.write(DecToBcd(tDateTime.DayOfWeek));
      uint8_t tMonthReg = DecToBcd(tDateTime.Month);
      if (tDateTime.Year >= 2100) tMonthReg |= 0x80;
      mWire.write(tMonthReg);
      mWire.write(DecToBcd((tDateTime.Year >= 2100 ? tDateTime.Year - 2100 : tDateTime.Year - 2000)));
      if (mWire.endTransmission() == 0) {
        xLOG("RTC write OK @ attempt %u/%u", tRetry + 1, I2C_RETRY_COUNT);
        return true;
      }
      if (tRetry < I2C_RETRY_COUNT - 1) {
        xLOG("RTC write attempt %u/%u failed, retrying...", tRetry + 1, I2C_RETRY_COUNT);
        vTaskDelay(pdMS_TO_TICKS(I2C_RETRY_DELAY_MS));
      }
    }
    xLOG("RTC write failed after %u attempts", I2C_RETRY_COUNT);
    return false;
  }

  bool RTC_::GetDateTime(SRTCDateTime &tDateTime) {
    Guard tLock;
    if (!mAvailable) return false;
    return ReadDateTimeWithRetry(tDateTime);
  }

  bool RTC_::SetDateTime(const SRTCDateTime &tDateTime) {
    Guard tLock;
    if (!mAvailable) return false;
    return WriteDateTimeWithRetry(tDateTime);
  }

  uint8_t RTC_::BcdToDec(uint8_t tBcd) {
    return ((tBcd >> 4) * 10) + (tBcd & 0x0F);
  }

  uint8_t RTC_::DecToBcd(uint8_t tDec) {
    return ((tDec / 10) << 4) | (tDec % 10);
  }

  unsigned long RTC_::DateTimeToEpoch(const SRTCDateTime &tDateTime) {
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
    return tDays * 86400UL + tDateTime.Hour * 3600UL + tDateTime.Minute * 60UL + tDateTime.Second;
  }

  void RTC_::EpochToDateTime(unsigned long tEpoch, SRTCDateTime &tDateTime) {
    unsigned long tSeconds = tEpoch % 60;
    tEpoch /= 60;
    unsigned long tMinutes = tEpoch % 60;
    tEpoch /= 60;
    unsigned long tHours = tEpoch % 24;
    tEpoch /= 24;
    tDateTime.Second = tSeconds;
    tDateTime.Minute = tMinutes;
    tDateTime.Hour = tHours;
    unsigned long tDays = tEpoch;
    tDateTime.DayOfWeek = ((tDays + 4) % 7);
    uint16_t tYear = 1970;
    while (true) {
      uint16_t tDaysInYear = (tYear % 4 == 0 && (tYear % 100 != 0 || tYear % 400 == 0)) ? 366 : 365;
      if (tDays < tDaysInYear) break;
      tDays -= tDaysInYear;
      tYear++;
    }
    tDateTime.Year = tYear;
    static const uint8_t tDaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t tMonth = 1;
    while (tMonth <= 12) {
      uint8_t tDim = tDaysInMonth[tMonth - 1];
      if (tMonth == 2 && (tYear % 4 == 0 && (tYear % 100 != 0 || tYear % 400 == 0))) tDim = 29;
      if (tDays < tDim) break;
      tDays -= tDim;
      tMonth++;
    }
    tDateTime.Month = tMonth;
    tDateTime.Day = tDays + 1;
  }

  unsigned long RTC_::GetEpoch() {
    Guard tLock;
    SRTCDateTime tDateTime;
    if (!GetDateTime(tDateTime)) return 0;
    return DateTimeToEpoch(tDateTime);
  }

  bool RTC_::SetFromEpoch(unsigned long tEpoch) {
    Guard tLock;
    SRTCDateTime tDateTime;
    EpochToDateTime(tEpoch, tDateTime);
    return SetDateTime(tDateTime);
  }

  bool RTC_::SyncFromNTP() {
    Guard tLock;
    unsigned long tEpoch = NTP.GetCurrentEpochUTC();
    if (tEpoch == 0) {
      xLOG("NTP time not available");
      return false;
    }
    bool tOk = SetFromEpoch(tEpoch);
    if (tOk) xLOG("RTC synced from → NTP");
    return tOk;
  }

  bool RTC_::SyncToSystem() {
    Guard tLock;
    unsigned long tEpoch = GetEpoch();
    if (tEpoch == 0) {
      xLOG("RTC epoch is 0 → cannot sync");
      return false;
    }
    if (tEpoch > 2147483647UL) {
      xLOG("RTC epoch %lu exceeds 32-bit signed max (Y2038 problem) → cannot sync", tEpoch);
      return false;
    }
    if (tEpoch < 1735689600UL) {
      xLOG("RTC epoch %lu is before 2026 → invalid", tEpoch);
      return false;
    }
    struct timeval tTv;
    tTv.tv_sec = (time_t)tEpoch;
    tTv.tv_usec = 0;
    if (settimeofday(&tTv, nullptr) != 0) {
      xLOG("settimeofday failed");
      return false;
    }
    xLOG("System time synced from → RTC");
    return true;
  }

  bool RTC_::SyncFromSystem() {
    Guard tLock;
    struct timeval tTv;
    gettimeofday(&tTv, nullptr);
    if (tTv.tv_sec < 1735689600) return false;
    bool tOk = SetFromEpoch(tTv.tv_sec);
    if (tOk) xLOG("RTC synced from → system time");
    return tOk;
  }

  void RTC_::GetTime(char *tBuffer, size_t tSize) {
    SRTCDateTime tDateTime;
    if (!GetDateTime(tDateTime)) {
      snprintf(tBuffer, tSize, "--:--:--");
      return;
    }
    snprintf(tBuffer, tSize, "%02d:%02d:%02d", tDateTime.Hour, tDateTime.Minute, tDateTime.Second);
  }

  void RTC_::GetDate(char *tBuffer, size_t tSize) {
    SRTCDateTime tDateTime;
    if (!GetDateTime(tDateTime)) {
      snprintf(tBuffer, tSize, "----.--.--");
      return;
    }
    snprintf(tBuffer, tSize, "%04d.%02d.%02d", tDateTime.Year, tDateTime.Month, tDateTime.Day);
  }

  void RTC_::GetDateTime(char *tBuffer, size_t tSize) {
    SRTCDateTime tDateTime;
    if (!GetDateTime(tDateTime)) {
      snprintf(tBuffer, tSize, "----.--.-- --:--:--");
      return;
    }
    snprintf(tBuffer, tSize, "%04d.%02d.%02d %02d:%02d:%02d", tDateTime.Year, tDateTime.Month, tDateTime.Day, tDateTime.Hour, tDateTime.Minute, tDateTime.Second);
  }

  void RTC_::PrintInfo() {
    SRTCDateTime tDt;
    if (GetDateTime(tDt)) {
      xLOG("RTC DateTime → %04d.%02d.%02d %02d:%02d:%02d", tDt.Year, tDt.Month, tDt.Day, tDt.Hour, tDt.Minute, tDt.Second);
    } else xLOG("RTC DateTime → read failed");
  }

}
