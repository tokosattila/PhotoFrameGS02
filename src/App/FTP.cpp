#include <App/FTP.h>

namespace App {

  char FTP_::mFileName[128] = "";

  FTP_ &FTP_::Instance() {
    static FTP_ tInstance;
    return tInstance;
  }

  FTP_::FTP_() {
    mMutex = xSemaphoreCreateRecursiveMutex();
    mCfg = CFG.Get<SAppConfig>();
    mFTP = FtpServer(mCfg.Ftp.FtpPort);
    mFTP.setCallback(SEventCallback);
    mFTP.setTransferCallback(STransferCallback);
  }

  FTP_::~FTP_() {
    End();
    if (mMutex) {
      vSemaphoreDelete(mMutex);
      mMutex = nullptr;
    }
  }

  void FTP_::Lock() {
    if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
  }

  void FTP_::Unlock() {
    if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
  }

  void FTP_::SEventCallback(FtpOperation tOperation, uint32_t tFreeSpace, uint32_t tTotalSpace) {
    FTP_ &tSelf = Instance();
    Guard tLock;
    switch (tOperation) {
      case FTP_CONNECT:
        xLOG("Client connected!");
        break;
      case FTP_DISCONNECT:
        xLOG("Client disconnected!");
        break;
      case FTP_FREE_SPACE_CHANGE:
        char tFreeSspaceBuffer[16];
        char tTotalSpaceBuffer[16];
        UTL.ByteToReadableSize(tFreeSpace, tFreeSspaceBuffer, sizeof(tFreeSspaceBuffer));
        UTL.ByteToReadableSize(tTotalSpace, tTotalSpaceBuffer, sizeof(tTotalSpaceBuffer));
        xLOG("FS space changed → %s / %s", tFreeSspaceBuffer, tTotalSpaceBuffer);
        break;
      default:
        break;
    }
    if (tSelf.mEventCallback) tSelf.mEventCallback(tOperation, tFreeSpace, tTotalSpace);
  }

  void FTP_::STransferCallback(FtpTransferOperation tOperation, const char *tFileName, uint32_t tFileSize) {
    FTP_ &tSelf = Instance();
    Guard tLock;
    switch (tOperation) {
      case FTP_UPLOAD_START:
        xLOG("Transfer start!");
        break;
      case FTP_UPLOAD:
        if (tFileName != nullptr && tFileName[0] != '\0') {
          strncpy(mFileName, tFileName, sizeof(mFileName) - 1);
          mFileName[sizeof(mFileName) - 1] = '\0';
        }
        char tFileSizeBuffer[16];
        UTL.ByteToReadableSize(tFileSize, tFileSizeBuffer, sizeof(tFileSizeBuffer));
        xLOG("Upload → %s [%s]", tSelf.mFileName, tFileSizeBuffer);
        break;
      case FTP_TRANSFER_STOP:
        xLOG("Transfer finish!");
        break;
      case FTP_TRANSFER_ERROR:
        xLOG("Transfer error!");
        break;        
      default:
        break;
    }
    if (tSelf.mCallback && tOperation == FTP_TRANSFER_STOP) tSelf.mCallback(tSelf.mFileName, tFileSize);
    if (tSelf.mTransferCallback) tSelf.mTransferCallback(tOperation, tSelf.mFileName, tFileSize);
  }

  bool FTP_::Init(bool tVerbose) {
    ReloadConfig();
    Guard tLock;
    bool tCanStart = CON.HasActiveWifiClient() && STG.IsMounted();
    if (tCanStart) {
      mFTP.begin(mCfg.Ftp.Username.c_str(), mCfg.Ftp.Password.c_str(), "Welcome to FTP Server");
      if (tVerbose) {
        xLOG("FTP init succesful!");
        xLOG("FTP started → ftp %s %d", CON.GetIpAddress(), mCfg.Ftp.FtpPort.Get());
      }
    } else {
      if (tVerbose) xLOG("FTP init failed!");
    }
    return tCanStart;
  }

  void FTP_::ReloadConfig() {
    Guard tLock;
    mCfg = CFG.Get<SAppConfig>();
  }

  void FTP_::End() {
    Guard tLock;
    mFTP.end();
  }

  void FTP_::HandleEvents() {
    Guard tLock;
    mFTP.handleFTP();
  }

  void FTP_::Callback(FFtpCallback tCallback) {
    Guard tLock;
    mCallback = tCallback;
  }

  void FTP_::EventCallback(FFtpEventCallback tCallback) {
    Guard tLock;
    mEventCallback = tCallback;
  }

  void FTP_::TransferCallback(FFtpTransferCallback tCallback) {
    Guard tLock;
    mTransferCallback = tCallback;
  }

}
