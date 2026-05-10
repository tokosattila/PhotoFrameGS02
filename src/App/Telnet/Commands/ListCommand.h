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
            tClient.print(F(COLOR_RED "\r\n  SD Card is not mounted.\r\n\r\n" COLOR_WHITE));
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
        if (strcasecmp(tArg, "logs") == 0) {
          return PrintLogsListing(tClient);
        }
        tClient.printf(COLOR_RED "\r\n  Unknown target: %s\r\n" COLOR_WHITE, tArg);
        tClient.print(F(COLOR_YELLOW "  Usage: list [sd|sdcard|lfs|littlefs|fallback|logs]\r\n\r\n" COLOR_WHITE));
        return true;
      }
      const char *Help() const override {
        return "list                              " COLOR_YELLOW "- list active storage\r\n  " COLOR_WHITE
               "list sd|sdcard                    " COLOR_YELLOW "- list SD Card\r\n  " COLOR_WHITE
               "list lfs|littlefs|fallback        " COLOR_YELLOW "- list LittleFS\r\n  " COLOR_WHITE
               "list logs                         " COLOR_YELLOW "- list logs directory tree" COLOR_WHITE;
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
        if (!tData || tLen == 0 || tData[0] == '\0') {
          tClient.print(F(COLOR_YELLOW "  File structure is empty.\r\n" COLOR_WHITE));
          return true;
        }
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
      bool PrintLogsListing(WiFiClient &tClient) {
        File tLogsDir;
        const char *tStorageLabel = "";
        tLogsDir = STG.OpenFile("/logs", FILE_READ);
        if (tLogsDir) tStorageLabel = STG.GetActiveName();
        else {
          if (STG.IsSDCard() && LFS.IsMounted()) {
            tLogsDir = LFS.OpenFile("/logs", FILE_READ);
            if (tLogsDir) tStorageLabel = "LittleFS (fallback)";
          } else if (STG.IsLittleFS() && SDC.IsMounted()) {
            tLogsDir = SDC.OpenFile("/logs", FILE_READ);
            if (tLogsDir) tStorageLabel = "SD Card (fallback)";
          }
        }
        if (!tLogsDir) {
          tClient.print(F(COLOR_YELLOW "  No logs directory found in active or fallback storage.\r\n\r\n" COLOR_WHITE));
          return true;
        }
        tClient.printf(COLOR_GREEN "\r\n  File structure [logs/ directory]:\r\n\r\n" COLOR_WHITE);
        if (tStorageLabel[0] != '\0') tClient.printf(COLOR_YELLOW "  Source: %s\r\n\r\n" COLOR_WHITE, tStorageLabel);
        PrintLogsRecursive(tLogsDir, "", 0, tClient);
        tLogsDir.close();
        tClient.print(F("\r\n" COLOR_WHITE));
        return true;
      }
      void PrintLogsRecursive(File tDir, const char *tIndent, uint8_t tDepth, WiFiClient &tClient) {
        char tIndentBuffer[128] = "";
        strncpy(tIndentBuffer, tIndent, sizeof(tIndentBuffer) - 1);
        File tFile = tDir.openNextFile();
        while (tFile) {
          const char *tName = tFile.name();
          if (tFile.isDirectory()) {
            tClient.printf("  %s%s/\r\n", tIndentBuffer, tName);
            File tSubDir = tFile;
            char tNewIndent[128] = "";
            snprintf(tNewIndent, sizeof(tNewIndent), "%s  ", tIndentBuffer);
            PrintLogsRecursive(tSubDir, tNewIndent, tDepth + 1, tClient);
            tSubDir.close();
          } else {
            uint32_t tSize = tFile.size();
            char tSizeBuf[32];
            Utils_::ByteToReadableSize(tSize, tSizeBuf, sizeof(tSizeBuf));
            tClient.printf("  %s%s [%s]\r\n", tIndentBuffer, tName, tSizeBuf);
          }
          tFile.close();
          tFile = tDir.openNextFile();
        }
      }
  };

}

#endif