#ifndef UTILS_H
#define UTILS_H

#include <App/Global.h>

namespace App {

  enum class EUtilsInfoType : uint8_t { 
    Header = 1, 
    Title, 
    Cell,
    Line,
    Footer,
    Single
  };

  class Utils_ {
    DEFINE_TAG("CFG");
    friend class AutoGuard<Utils_>;
    public:
      using Guard = AutoGuard<Utils_>;
      static Utils_ &Instance();
      uint8_t mBatteryPercentage = 100;
      float mBatteryVoltage;
      void Init();
      void ReloadConfig();
      static void SetCPUFrequency(ECPUFrequency tFrequency = ECPUFrequency::F240MHz);
      static void DisableBT();
      static void DisableTouchPad();
      static void DisableBrownout();
      static void ByteToReadableSize(uint32_t tBytes, char *tBuffer, size_t tLength);
      const char *EpochToReadableFormat(unsigned long tEpoch, bool tAsDateTime, char *tBuffer, size_t tLength);
      void PrintBootInfo();
      void PrintWakeupReason();
      void PrintDeviceInfo();
      void PrintInfo(const char *tText, EUtilsInfoType tType = EUtilsInfoType::Cell, uint8_t tWidth = 0);
      void PrintMemoryInfo();
      void SetPrintInfoWidth(uint8_t tWidth) { mPrintInfoWidth = tWidth; }
      uint8_t GetPrintInfoWidth() const { return mPrintInfoWidth; }
      const char *PrependSlash(const char *tPath, char *tOutBuffer, size_t tBufSize);
      void SleepLowBattery();
      void SleepAndWakeup();
      bool MeasureBattery();
      static bool WasWokenByButton();
      static bool SecureStrcmp(const char *tA, const char *tB);
      static uint32_t SafeAtoul(const char *tStr, uint32_t tMinVal, uint32_t tMaxVal, uint32_t tDefaultVal);
    private:
      Utils_();
      Utils_(const Utils_&) = delete;
      Utils_ &operator=(const Utils_&) = delete;
      ~Utils_(); 
      mutable SemaphoreHandle_t mMutex;
      SAppConfig mCfg {};
      uint8_t mPrintInfoWidth = 43;
      uint16_t mVref = 1100;
      static void Lock();
      static void Unlock();
      void PrintChipInfo();
      void PrintFlashInfo();
      void PrintNvsUsageInfo();
      void PrintRamInfo();
      void PrintDRamUsageInfo();
      void PrintIRamUsageInfo();
      void PrintPSRamInfo();
      void PrintPSRamUsageInfo();
      void PrintSketchInfo();
      void PrintFileSystemInfo();
      void PrintResourceInfo();
      void PrintRadioInfo();
  };

}

#endif
