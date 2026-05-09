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
      using FConfirmCallback = std::function<void(bool tConfirmed, WiFiClient &tClient)>;
      using FActivityCallback = std::function<void()>;
      bool Init(bool tVerbose = false);
      void End();
      void ReloadConfig();
      void RegisterCommand(Command_ *tCommand);
      void HandleEvents();
      void SaveSessionTimestamp();
      void ClearSession();
      void RequestConfirmation(const char *tPrompt, FConfirmCallback tCallback);
      void ActivityCallback(FActivityCallback tCallback);
    private:
      Telnet_();
      Telnet_(const Telnet_&) = delete;
      Telnet_ &operator=(const Telnet_&) = delete;
      ~Telnet_();
      WiFiServer mServer {};
      WiFiClient mClient {};
      mutable SemaphoreHandle_t mMutex = nullptr;
      FConnectionCallback mCallback = nullptr;
      FConfirmCallback mConfirmCallback = nullptr;
      FActivityCallback mActivityCallback = nullptr;
      volatile bool mEnabled = false;
      bool mAuthRequired = true;
      uint32_t mAuthTimestamp = 0;
      uint8_t mInputPos = 0;
      bool mWaitingConfirmation = false;
      char mConfirmPrompt[96] = "Confirm (y/n): ";
      uint8_t mFailedAttempts = 0;
      uint8_t mLockoutLevel = 0;
      uint32_t mLockoutUntil = 0;
      static constexpr uint8_t kMaxAttemptsPerLevel = 3;
      static constexpr uint32_t kLockoutDurations[] = {30000, 3600000, 86400000};
      static constexpr uint8_t kMaxLockoutLevel = 2;
      bool IsLockedOut();
      void ApplyLockout();
      void ResetLockout();
      static void Lock();
      static void Unlock();
      void ClearScreen();
      bool IfHaveArguments(const char *input);
      bool IsAuthenticated();
      const char *GetCurrentPrompt();
      void WritePrompt();   
      bool ParseYesNo(const char *tInput, bool &tValue) const;
      void NotifyActivity();
  };

}

#endif
