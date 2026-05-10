#ifndef EXIT_COMMAND_CLASS
#define EXIT_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class ExitCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "exit"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        tClient.print(F(COLOR_RED "\r\nConnection closed. Type 'exit' to close this window.\r\n" COLOR_WHITE));
        TLN.ClearSession();
        TLN.mWaitingPassword = false;
        TLN.mExitRequested = true;
        vTaskDelay(DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
        tClient.stop();
        return true;
      }
      const char *Help() const override {
        return "exit                              - exit telnet";
      }
  };

}

#endif