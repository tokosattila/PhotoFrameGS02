#include <App/Telnet.h>
#include <App/Telnet/Commands/HelpCommand.h>
#include <App/Telnet/Commands/ClearCommand.h>
#include <App/Telnet/Commands/ListCommand.h>
#include <App/Telnet/Commands/CatCommand.h>
#include <App/Telnet/Commands/DateCommand.h>
#include <App/Telnet/Commands/TimeStampCommand.h>
#include <App/Telnet/Commands/NvsInfoCommand.h>
#include <App/Telnet/Commands/MemInfoCommand.h>
#include <App/Telnet/Commands/SketchInfoCommand.h>
#include <App/Telnet/Commands/FileSystemInfoCommand.h>
#include <App/Telnet/Commands/NetInfoCommand.h>
#include <App/Telnet/Commands/BatInfoCommand.h>
#include <App/Telnet/Commands/ConfigCommand.h>
#include <App/Telnet/Commands/FetchCommand.h>
#include <App/Telnet/Commands/FirmwareUpdateCommand.h>
#include <App/Telnet/Commands/BootPartitionCommand.h>
#include <App/Telnet/Commands/CallbackCommand.h>
#include <App/Telnet/Commands/ResetCommand.h>
#include <App/Telnet/Commands/RebootCommand.h>
#include <App/Telnet/Commands/LogoutCommand.h>
#include <App/Telnet/Commands/ExitCommand.h>
#include <App/Telnet/Commands/NotFoundCommand.h>

namespace App {

  static HelpCommand_ sHelpCmd;
  static ClearCommand_ sClearCmd;
  static ListCommand_ sListCmd;
  static CatCommand_ sCatCmd;
  static DateCommand_ sDateCmd;
  static TimeStampCommand_ sTimeStampCmd;
  static NvsInfoCommand_ sNvsInfoCmd;
  static MemInfoCommand_ sMemInfoCmd;
  static SketchInfoCommand_ sSketchInfoCmd;
  static FileSystemInfoCommand_ sFileSystemInfoCmd;
  static NetInfoCommand_ sNetInfoCmd;
  static BatInfoCommand_ sBatInfoCmd;
  static ConfigCommand_ sConfigCmd;
  static FetchCommand_ sFetchCmd;
  static FirmwareUpdateCommand_ sFirmwareUpdateCmd;
  static BootPartitionCommand_ sBootPartitionCmd;
  static ResetCommand_ sResetCmd;
  static RebootCommand_ sRebootCmd;
  static LogoutCommand_ sLogoutCmd;
  static ExitCommand_ sExitCmd;
  static NotFoundCommand_ sNotFoundCmd; 

  constexpr uint16_t kBufferSize = sizeof(Telnet_::Instance().mInputBuffer);

  Telnet_ &Telnet_::Instance() {
    static Telnet_ tInstance;
    return tInstance;
  }

  Telnet_::Telnet_() {
    mMutex = xSemaphoreCreateRecursiveMutex();
    mCfg = CFG.Get<SAppConfig>();
    mServer = WiFiServer(mCfg.Telnet.TelnetPort);
    mEnabled = false;
    mAuthRequired = true;
    mWaitingPassword = false;
    mAuthTimestamp = 0;
    mInputPos = 0;
    memset(mInputBuffer, 0, sizeof(mInputBuffer));
    mCommands.reserve(20);
  }

  Telnet_::~Telnet_() {
    if (mMutex) {
      vSemaphoreDelete(mMutex);
      mMutex = nullptr;
    }
  }

  void Telnet_::Lock() {
    if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
  }

  void Telnet_::Unlock() {
    if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
  }

  bool Telnet_::Init(bool tVerbose) {
    ReloadConfig();
    Guard tLock;
    bool tCanStart = CON.HasActiveWifiClient() && STG.IsMounted() && mCfg.Telnet.Enable;
    if (tCanStart) {
      mServer.begin(mCfg.Telnet.TelnetPort);
      mServer.setNoDelay(true);
      mEnabled = true;
      RegisterCommand(&sHelpCmd);
      RegisterCommand(&sClearCmd);
      RegisterCommand(&sListCmd);
      RegisterCommand(&sCatCmd);
      RegisterCommand(&sDateCmd);
      RegisterCommand(&sTimeStampCmd);
      RegisterCommand(&sNvsInfoCmd);
      RegisterCommand(&sMemInfoCmd);
      RegisterCommand(&sSketchInfoCmd);
      RegisterCommand(&sFileSystemInfoCmd);
      RegisterCommand(&sNetInfoCmd);
      RegisterCommand(&sBatInfoCmd);
      RegisterCommand(&sConfigCmd);
      RegisterCommand(&sFetchCmd);
      RegisterCommand(&sFirmwareUpdateCmd);
      RegisterCommand(&sBootPartitionCmd);
      RegisterCommand(&sResetCmd);
      RegisterCommand(&sRebootCmd);
      RegisterCommand(&sLogoutCmd);
      RegisterCommand(&sExitCmd);
      RegisterCommand(&sNotFoundCmd);
      mAuthTimestamp = CFG.GetSession();
      if (tVerbose) {
        xLOG("Telnet init succesful!");
        xLOG("Telnet started → telnet %s %d", CON.GetIpAddress(), mCfg.Telnet.TelnetPort.Get());
      }
    } else {
      if (tVerbose) xLOG("Telnet init failed!");
      mEnabled = false;
      return false;
    }
    return true;
  }

  void Telnet_::End() {
    Guard tLock;
    if (mClient) {
      mClient.stop();
      mClient = WiFiClient();
    }
    mServer.stop();
    mEnabled = false;
    mAuthTimestamp = 0;
  }

  void Telnet_::ReloadConfig() {
    Guard tLock;
    mCfg = CFG.Get<SAppConfig>();
  }

  void Telnet_::RegisterCommand(Command_ *tCommand) {
    Guard tLock;
    if (tCommand != nullptr) mCommands.push_back(tCommand);
  }

  void Telnet_::HandleEvents() {
    Guard tLock;
    if (!mEnabled) return;
    if (mExitRequested) {
      if (mClient) mClient.stop();
      mExitRequested = false;
      return;
    }
    if (mServer.hasClient()) {
      if (!mClient || !mClient.connected()) {
        if (mClient) mClient.stop();
        mClient = mServer.available();
        mClient.setNoDelay(true);
        mInputPos = 0;
        memset(mInputBuffer, 0, sizeof(mInputBuffer));
        if (!IsAuthenticated() && mAuthRequired) {
          mClient.printf("%s TELNET %s\r\n\r\n" COLOR_YELLOW "Authentication required!" COLOR_WHITE "\r\n\r\nUsername: ", mCfg.Device.Name.c_str(), mCfg.Device.Version.c_str());
          mWaitingPassword = false;
        } else {
          ClearScreen();
          mClient.print(F(COLOR_YELLOW "Type 'help' to list commands." COLOR_WHITE "\r\n\r\n$ "));
        }
        while (mClient.available()) mClient.read();
      } else mServer.available().stop();
    }
    if (mClient && mClient.connected() && mClient.available()) {
      while (mClient.available()) {
        char tC = mClient.read();
        if (tC == '\r' || tC == '\n') {
          if (tC == '\r' && mClient.peek() == '\n') mClient.read(); 
          if (mInputPos == 0) {
            WritePrompt();
            break;
          }
          mInputBuffer[mInputPos] = '\0';
          bool tAuthenticated = IsAuthenticated();
          if (!tAuthenticated  && mAuthRequired) {
            if (!mWaitingPassword) {
              if (UTL.SecureStrcmp(mInputBuffer, mCfg.Telnet.Username.c_str())) {
                mClient.print(F("Password: "));
                mWaitingPassword = true;
              } else mClient.print(F("\r\n" COLOR_RED "Invalid username." COLOR_WHITE "\r\n\r\nUsername: "));
            } else {
              if (UTL.SecureStrcmp(mInputBuffer, mCfg.Telnet.Password.c_str())) {
                SaveSessionTimestamp();
                ClearScreen();
                mInputPos = 0;
                memset(mInputBuffer, 0, sizeof(mInputBuffer));
                mClient.printf("%s TELNET\r\n\r\n" COLOR_YELLOW "Welcome! Type 'help' for commands." COLOR_WHITE "\r\n\r\n$ ", mCfg.Device.Name.c_str());
                mWaitingPassword = false;
                break;
              } else mClient.print(F("\r\n" COLOR_RED "Invalid password." COLOR_WHITE "\r\n\r\nPassword: "));
            }
            mInputPos = 0;
            memset(mInputBuffer, 0, sizeof(mInputBuffer));
            WritePrompt();
            continue;
          } 
          if (mWaitingConfirmation) {
            bool tConfirmed = false;
            if (!ParseYesNo(mInputBuffer, tConfirmed)) {
              mClient.print(F("\r\n" COLOR_YELLOW "  Please answer 'y' or 'n' (or 'cancel')" COLOR_WHITE "\r\n"));
              mInputPos = 0;
              memset(mInputBuffer, 0, sizeof(mInputBuffer));
              WritePrompt();
              continue;
            }
            auto tCallback = mConfirmCallback;
            mWaitingConfirmation = false;
            mConfirmCallback = nullptr;
            mInputPos = 0;
            memset(mInputBuffer, 0, sizeof(mInputBuffer));
            if (tCallback) tCallback(tConfirmed, mClient);
            WritePrompt();
            continue;
          }
          char tCmdName[128] = "";
          strncpy(tCmdName, mInputBuffer, sizeof(tCmdName) - 1);
          char *tSpace = strchr(tCmdName, ' ');
          if (tSpace) *tSpace = '\0';
          bool tHandled = false;
          for (Command_ *tCmd : mCommands) {
            if (strcasecmp(tCmdName, tCmd->GetName()) == 0) {
              tCmd->Execute(mInputBuffer, mClient);
              tHandled = true;
              break;
            }
          }
          if (!tHandled) {
            for (Command_ *tCmd : mCommands) {
              if (strcasecmp(tCmd->GetName(), "notfound") == 0) {
                tCmd->Execute(mInputBuffer, mClient);
                break;
              }
            }
          }
          mInputPos = 0;
          memset(mInputBuffer, 0, sizeof(mInputBuffer));
          WritePrompt();
          continue;
        }
        if (tC == 0x08 || tC == 0x7F) {
          if (mInputPos > 0) {
            mInputPos--;
            mInputBuffer[mInputPos] = '\0';
          }
          WritePrompt();
          continue;  
        }
        if (isprint(tC) && mInputPos < sizeof(mInputBuffer) - 2) { 
          mInputBuffer[mInputPos++] = tC;
          mInputBuffer[mInputPos] = '\0';
          if (!IsAuthenticated() && mWaitingPassword) mClient.write("\u0008*");
          continue;
        }
      }
    }
  }

  bool Telnet_::IsAuthenticated() {
    if (!mAuthRequired || !mEnabled) return true;
    if (mAuthTimestamp == 0) {
      Guard tLock;
      mAuthTimestamp = CFG.GetSession();
      if (mAuthTimestamp == 0) return false;
    };
    uint32_t tNow = millis();
    uint32_t tSessionMs = static_cast<uint32_t>(mCfg.Telnet.Session) * 1000UL;
    if (tNow < mAuthTimestamp || (tNow - mAuthTimestamp) > tSessionMs) {
      ClearSession();
      return false;
    }
    return true;
  }

  void Telnet_::ClearScreen() {
    mClient.print(F("\x1B[2J\x1B[H"));
    vTaskDelay(DELAY_ULTRA_SHORT_MS / portTICK_PERIOD_MS);
  }

  bool Telnet_::IfHaveArguments(const char *tInput) {
    for (size_t i = 0; i < strlen(tInput); i++) {
      if (isWhitespace((int)tInput[i])) return true;
    }
    return false;
  }

  const char *Telnet_::GetCurrentPrompt() {
    if (!IsAuthenticated() && mAuthRequired) return mWaitingPassword ? "Password: " : "Username: ";
    if (mWaitingConfirmation) return mConfirmPrompt;
    return "$ ";
  }

  bool Telnet_::ParseYesNo(const char *tInput, bool &tValue) const {
    if (!tInput) return false;
    while (*tInput && isWhitespace((int)*tInput)) tInput++;
    if (*tInput == '\0') return false;
    if (*tInput == 'y' || *tInput == 'Y') {
      tValue = true;
      return true;
    }
    if (*tInput == 'n' || *tInput == 'N') {
      tValue = false;
      return true;
    }
    if (*tInput == 'c' || *tInput == 'C') {
      tValue = false;
      return true;
    }
    if (strcasecmp(tInput, "yes") == 0) {
      tValue = true;
      return true;
    }
    if (strcasecmp(tInput, "no") == 0) {
      tValue = false;
      return true;
    }
    if (strcasecmp(tInput, "cancel") == 0 || strcasecmp(tInput, "abort") == 0) {
      tValue = false;
      return true;
    }
    return false;
  }

  void Telnet_::RequestConfirmation(const char *tPrompt, FConfirmCallback tCallback) {
    Guard tLock;
    if (!tPrompt || !tPrompt[0]) tPrompt = "Confirm (y/n): ";
    snprintf(mConfirmPrompt, sizeof(mConfirmPrompt), "  %s", tPrompt);
    mConfirmCallback = std::move(tCallback);
    mWaitingConfirmation = true;
    mInputPos = 0;
    memset(mInputBuffer, 0, sizeof(mInputBuffer));
    WritePrompt();
  }

  void Telnet_::WritePrompt() {
    mClient.print(F("\r\x1B[K"));
    mClient.print(GetCurrentPrompt());
    if (!IsAuthenticated() && mWaitingPassword) mClient.print(String('*', mInputPos));
    else mClient.print(mInputBuffer);
  }

  void Telnet_::SaveSessionTimestamp() {
    Guard tLock;
    if (!CFG.SaveSession(millis())) {
      xLOG("Warning: Failed to save session timestamp");
    }
  }

  void Telnet_::ClearSession() {
    Guard tLock;
    mAuthTimestamp = 0;
    if (!CFG.SaveSession(mAuthTimestamp)) {
      xLOG("Warning: Failed to clear session timestamp");
    }
  }

}
