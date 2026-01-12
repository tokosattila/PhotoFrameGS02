#ifndef LOGOUT_COMMAND_CLASS
#define LOGOUT_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class LogoutCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "logout"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        TLN.ClearSession();
        TLN.mWaitingPassword = false;
        tClient.print(F("\u001B[2J"));
        tClient.printf("%s TELNET\r\n\r\n" COLOR_YELLOW "Authentication required!" COLOR_WHITE "\r\n\r\nUsername: ", TLN.mCfg.Device.Name);
        return true;
      }
      const char *Help() const override {
        return "logout                           - logout telnet session";
      }
  };

}

#endif