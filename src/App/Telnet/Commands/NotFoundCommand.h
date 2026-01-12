#ifndef NOT_FOUND_COMMAND_CLASS
#define NOT_FOUND_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class NotFoundCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "notfound"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        tClient.printf("\r\n" COLOR_RED "  Command not found: %s" COLOR_WHITE "\r\n\r\n", tArguments);
        return true;
      }
      const char *Help() const override {
        return "";
      }
  };

}

#endif