#ifndef REBOOT_COMMAND_CLASS
#define REBOOT_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class RebootCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "reboot"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        tClient.print(F("\r\n" COLOR_RED "  Rebooting system..." COLOR_WHITE));
        vTaskDelay(DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
        TLN.ClearSession();
        TLN.mWaitingPassword = false;
        TLN.mExitRequested = true;
        ESP.restart();
        return true;
      }
      const char *Help() const override {
        return "reboot                           - restart device";
      }
  };

}

#endif