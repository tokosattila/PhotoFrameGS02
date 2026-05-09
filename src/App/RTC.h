#ifndef RTC_H
#define RTC_H

#include <App/Global.h>
#include <Wire.h>

namespace App {

  struct SRTCDateTime {
    uint8_t Second = 0;
    uint8_t Minute = 0;
    uint8_t Hour = 0;
    uint8_t DayOfWeek = 0;
    uint8_t Day = 1;
    uint8_t Month = 1;
    uint16_t Year = 2026;
  };

  class RTC_ {
    DEFINE_TAG("RTC");
    friend class AutoGuard<RTC_>;
    public:
      using Guard = AutoGuard<RTC_>;
      static RTC_ &Instance();
      bool Init(bool tVerbose = false);
      void End();
      bool IsAvailable() const { return mAvailable; }
      bool GetDateTime(SRTCDateTime &tDateTime);
      bool SetDateTime(const SRTCDateTime &tDateTime);
      unsigned long GetEpoch();
      bool SetFromEpoch(unsigned long tEpoch);
      bool SyncFromNTP();
      bool SyncToSystem();
      bool SyncFromSystem();
      void GetTime(char *tBuffer, size_t tSize);
      void GetDate(char *tBuffer, size_t tSize);
      void GetDateTime(char *tBuffer, size_t tSize);
      void PrintInfo();
    private:
      RTC_();
      RTC_(const RTC_&) = delete;
      RTC_ &operator=(const RTC_&) = delete;
      ~RTC_();
      TwoWire mWire = TwoWire(0);
      const uint8_t mSdaPin = RTC_SDA_PIN;
      const uint8_t mSclPin = RTC_SCL_PIN;
      const uint8_t mAddress = RTC_ADDRESS;
      mutable SemaphoreHandle_t mMutex = nullptr;
      bool mAvailable = false;
      static constexpr uint8_t I2C_RETRY_COUNT = 3;
      static constexpr uint32_t I2C_RETRY_DELAY_MS = 5;
      static void Lock();
      static void Unlock();
      bool TryI2C();
      bool ReadDateTimeRaw(SRTCDateTime &tDateTime, bool &tVLFlagSet);
      bool ReadDateTimeWithRetry(SRTCDateTime &tDateTime);
      bool WriteDateTimeWithRetry(const SRTCDateTime &tDateTime);
      bool IsDateTimePlausible(const SRTCDateTime &tDateTime);
      static uint8_t BcdToDec(uint8_t tBcd);
      static uint8_t DecToBcd(uint8_t tDec);
      static unsigned long DateTimeToEpoch(const SRTCDateTime &tDateTime);
      static void EpochToDateTime(unsigned long tEpoch, SRTCDateTime &tDateTime);
  };

}

#endif
