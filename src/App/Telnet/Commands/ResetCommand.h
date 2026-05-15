#ifndef RESET_COMMAND_CLASS
#define RESET_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class ResetCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "reset"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        const char *tArg = tArguments ? strchr(tArguments, ' ') : nullptr;
        if (tArg) tArg++;
        if (tArg && (strcasecmp(tArg, "config") == 0)) {
          tClient.print(F(COLOR_YELLOW "\r\n  This will reset ALL settings to factory defaults!" COLOR_WHITE "\r\n  Reset config? (y/n): "));
          TLN.RequestConfirmation("", [](bool tConfirmed, WiFiClient &tConfirmClient) {
            if (!tConfirmed) {
              tConfirmClient.print(F("\r\n  Cancelled\r\n\r\n"));
              return;
            }
            tConfirmClient.print(F(COLOR_RED "\r\n  Resetting to factory defaults..." COLOR_WHITE "\r\n\r\n"));
            vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
            CFG.FactoryReset();
            tConfirmClient.print(F(COLOR_GREEN "  Config reset complete!\r\n\r\n" COLOR_RED "  Rebooting..." COLOR_WHITE "\r\n\r\n"));
            vTaskDelay(DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
            TLN.ClearSession();
            TLN.mWaitingPassword = false;
            TLN.mExitRequested = true;
            esp_restart();
          });
          return true;
        }
        tClient.print(F(COLOR_RED "\r\n  This will reset ALL settings to factory defaults!" COLOR_YELLOW "\r\n  Usage: reset config\r\n\r\n" COLOR_WHITE));
        return true;
      }
      const char *Help() const override {
        return "reset config                      - factory reset config (asks y/n)";
      }
  };

}

#endif
