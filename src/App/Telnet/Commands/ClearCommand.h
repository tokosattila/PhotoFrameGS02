#ifndef CLEAR_COMMAND_CLASS
#define CLEAR_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class ClearCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "clear"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        tClient.print(F("\033[2J\033[H"));
        return true;
      }
      const char *Help() const override {
        return "clear                             - clear display";
      }
  };

}

#endif