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
        const char *tArg = ParseSubcommand(tArguments);
        if (tArg[0] == '\0') return PrintListing(STG.ListDir("/"), STG.GetListPos(), STG.GetActiveName(), tClient);
        if (strcasecmp(tArg, "sd") == 0 || strcasecmp(tArg, "sdcard") == 0) {
          if (!SDC.IsMounted()) {
            tClient.print(F(COLOR_RED "\r\n  SD card is not mounted.\r\n\r\n" COLOR_WHITE));
            return true;
          }
          return PrintListing(SDC.ListDir("/"), SDC.GetListPos(), SDC.GetName(), tClient);
        }
        if (strcasecmp(tArg, "lfs") == 0 || strcasecmp(tArg, "littlefs") == 0 || strcasecmp(tArg, "fallback") == 0) {
          if (!LFS.IsMounted()) {
            tClient.print(F(COLOR_RED "\r\n  LittleFS is not mounted.\r\n\r\n" COLOR_WHITE));
            return true;
          }
          return PrintListing(LFS.ListDir("/"), LFS.GetListPos(), LFS.GetName(), tClient);
        }
        tClient.printf(COLOR_RED "\r\n  Unknown target: %s\r\n" COLOR_WHITE, tArg);
        tClient.print(F(COLOR_YELLOW "  Usage: list [sd|sdcard|lfs|littlefs|fallback]\r\n\r\n" COLOR_WHITE));
        return true;
      }
      const char *Help() const override {
        return "list                             " COLOR_YELLOW "- list active storage\r\n  " COLOR_WHITE
               "list sd|sdcard                   " COLOR_YELLOW "- list SD card\r\n  " COLOR_WHITE
               "list lfs|littlefs|fallback       " COLOR_YELLOW "- list LittleFS" COLOR_WHITE;
      }
    private:
      const char *ParseSubcommand(const char *tInput) {
        if (!tInput) return "";
        const char *tPtr = tInput;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        return tPtr;
      }
      bool PrintListing(const char *tData, size_t tLen, const char *tLabel, WiFiClient &tClient) {
        tClient.printf(COLOR_GREEN "\r\n  File structure [%s]:\r\n\r\n" COLOR_WHITE, tLabel);
        char *tLine = (char *)tData;
        char *tEnd = (char *)tData + tLen;
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
  };

}

#endif