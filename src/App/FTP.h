#ifndef FTP_H
#define FTP_H

#include <App/Global.h>

namespace App {

  using FFtpCallback = std::function<void(const char *tFileName, uint32_t tFileSize)>;
  using FFtpEventCallback = std::function<void(FtpOperation tOperation, uint32_t tFreeSpace, uint32_t tTotalSpace)>;
  using FFtpTransferCallback = std::function<void(FtpTransferOperation tOperation, const char *tFileName, uint32_t tFileSize)>;

  class FTP_ {
    DEFINE_TAG("FTP");
    friend class AutoGuard<FTP_>;
    public:
      using Guard = AutoGuard<FTP_>;
      static FTP_ &Instance();
      bool Init(bool tVerbose = false);
      void ReloadConfig();
      void End();
      void HandleEvents();
      void Callback(FFtpCallback tCallback);
      void EventCallback(FFtpEventCallback tCallback);
      void TransferCallback(FFtpTransferCallback tCallback);
    private:
      FTP_();
      FTP_(const FTP_ &) = delete;
      FTP_ &operator=(const FTP_ &) = delete;      
      ~FTP_();
      FtpServer mFTP {};
      mutable SemaphoreHandle_t mMutex = nullptr;
      SAppConfig mCfg {};
      FFtpCallback mCallback = nullptr;
      FFtpEventCallback mEventCallback = nullptr;
      FFtpTransferCallback mTransferCallback = nullptr;
      static char mFileName[128];
      static void Lock();
      static void Unlock();
      static void SEventCallback(FtpOperation tOperation, uint32_t tFreeSpace, uint32_t tTotalSpace);
      static void STransferCallback(FtpTransferOperation tOperation, const char *tFileName, uint32_t tFileSize);
  };

}

#endif
