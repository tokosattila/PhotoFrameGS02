#ifndef TELNET_CLASS
#define TELNET_CLASS

#include <App/Global.h>

namespace App {

  class Command_;
  class Telnet_;

  class Telnet_ {
    DEFINE_TAG("TLN");
    friend class AutoGuard<Telnet_>;
    public:
      using Guard = AutoGuard<Telnet_>;
      static Telnet_ &Instance();
      SAppConfig mCfg {};
      char mInputBuffer[128];
      std::vector<Command_*> mCommands;
      bool mExitRequested = false;
      bool mWaitingPassword = false;
      bool Init(bool tVerbose = false);
      void End();
      void ReloadConfig();
      void RegisterCommand(Command_ *tCommand);
      void HandleEvents();
      void SaveSessionTimestamp();
      void ClearSession();
    private:
      Telnet_();
      Telnet_(const Telnet_&) = delete;
      Telnet_ &operator=(const Telnet_&) = delete;
      ~Telnet_();
      WiFiServer mServer {};
      WiFiClient mClient {};
      mutable SemaphoreHandle_t mMutex = nullptr;
      FConnectionCallback mCallback = nullptr;
      volatile bool mEnabled = false;
      bool mAuthRequired = true;
      uint32_t mAuthTimestamp = 0;
      uint8_t mInputPos = 0;
      static void Lock();
      static void Unlock();
      void ClearScreen();
      bool IfHaveArguments(const char *input);
      bool IsAuthenticated();
      const char *GetCurrentPrompt();
      void WritePrompt();   
  };

}

#endif
