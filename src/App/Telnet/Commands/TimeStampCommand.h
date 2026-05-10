#ifndef TIMESTAMP_COMMAND_CLASS
#define TIMESTAMP_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class TimeStampCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "timestamp"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        time_t tNow;
        time(&tNow);
        tClient.print(F("\r\n  Timestamp: "));
        tClient.print((unsigned long)tNow);
        tClient.print(F("\r\n\r\n"));
        return true;
      }
      const char *Help() const override {
        return "timestamp                         - show current timestamp";
      }
  };

}

#endif