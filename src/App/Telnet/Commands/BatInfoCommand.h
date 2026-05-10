#ifndef BATINFO_COMMAND_CLASS
#define BATINFO_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class BatInfoCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "batinfo";
      }
      bool Execute(const char *tArguments, WiFiClient& tClient) override {
        tClient.print(F(COLOR_WHITE "\r\n"));
        if (!UTL.MeasureBattery()) {
          tClient.println(F(COLOR_RED "  Battery: LOW or not detected!\r\n" COLOR_WHITE));
          return false;
        }
        char tText[32];
        snprintf(tText, sizeof(tText), "  Battery: %.2fV [%d%%]", UTL.mBatteryVoltage, UTL.mBatteryPercentage);
        tClient.println(tText);
        tClient.print(F("\r\n"));
        return true;
      }
      const char *Help() const override {
        return "batinfo                           - show battery voltage and percentage";
      }
  };

}

#endif
