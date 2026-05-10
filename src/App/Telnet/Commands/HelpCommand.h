#ifndef HELP_COMMAND_CLASS
#define HELP_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class HelpCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "help"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        if (TLN.mCommands.empty()) {
          tClient.print(F(COLOR_RED "  No command(s) available." COLOR_WHITE "\r\n"));
          return true;
        } 
        tClient.print(F("\r\n" COLOR_GREEN "  Available commands:\r\n\r\n" COLOR_WHITE));
        for (Command_ *tCmd : TLN.mCommands) {
          const char *tHelp = tCmd->Help();
          if (tHelp && tHelp[0] != '\0') {
            const char *tDash = strstr(tHelp, " - ");
            if (tDash) {
              tClient.print("  ");
              tClient.write(tHelp, tDash - tHelp);
              tClient.printf(COLOR_YELLOW "%s\r\n" COLOR_WHITE, tDash);
            } else tClient.printf("  %s\r\n", tCmd->Help());
          } 
        }
        tClient.print(F(COLOR_WHITE "\r\n"));
        return true;
      }
      const char *Help() const override {
        return "help                              - help information";
      }
  };

}

#endif