#ifndef LIST_COMMAND_CLASS
#define LIST_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class ListCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "list"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        const char *tData = STG.ListDir("/");
        size_t tLen = STG.GetListPos();
        char *tLine = (char*)tData;
        char *tEnd = (char*)tData + tLen;
        tClient.print(F(COLOR_GREEN "\r\n  File structure:\r\n\r\n" COLOR_WHITE));
        while (tLine < tEnd) {
          char *tNext = tLine;
          while (tNext < tEnd && *tNext != '\r' && *tNext != '\n') ++tNext;
          char tTemp = *tNext; *tNext = '\0';
          tClient.printf("  %s\r\n", tLine);
          if (tTemp) *tNext = tTemp;
          tLine = tNext + (tTemp ? (tTemp == '\r' && tNext[1] == '\n' ? 2 : 1) : 0);
        }
        tClient.print(F("\r\n" COLOR_WHITE));
        return true;
      }
      const char *Help() const override {
        return "list                             - list directories and files";
      }
  };

}

#endif