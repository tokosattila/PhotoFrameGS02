#ifndef CAT_COMMAND_CLASS
#define CAT_COMMAND_CLASS
#include <App/Telnet/Command.h>
namespace App {

  class CatCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "cat";
      }
      bool Execute(const char *tArguments, WiFiClient& tClient) override {
        if (!tArguments || tArguments[0] == '\0') {
          tClient.print(F(COLOR_RED "\r\n  Error: Missing filename\r\n"));
          tClient.print(F("  Usage: cat <filename>\r\n\r\n" COLOR_WHITE));
          return true;
        }
        const char *tPtr = tArguments;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr == '\0') {
          tClient.print(F(COLOR_RED "\r\n  Error: No filename provided\r\n"));
          tClient.print(F(COLOR_YELLOW "  Usage: cat <filename>\r\n\r\n" COLOR_WHITE));
          return true;
        }
        const char *tFileStart = tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        size_t tFileLen = tPtr - tFileStart;
        if (tFileLen == 0 || tFileLen >= 128) {
          tClient.print(F(COLOR_RED "\r\n  Error: Invalid filename\r\n\r\n" COLOR_WHITE));
          return true;
        }
        char tFileName[128] = "";
        strncpy(tFileName, tFileStart, tFileLen);
        tFileName[tFileLen] = '\0';
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr != '\0') {
          tClient.print(F(COLOR_RED "\r\n  Error: Too many arguments\r\n"));
          tClient.print(F(COLOR_YELLOW "  Usage: cat <filename>\r\n\r\n" COLOR_WHITE));
          return true;
        }
        const char *tFullPath = STG.NormalizePath(tFileName);
        if (!STG.Exists(tFullPath)) {
          tClient.print(F(COLOR_RED "\r\n  Error: File not found: "));
          tClient.print(tFullPath);
          tClient.print(F("\r\n\r\n" COLOR_WHITE));
          return true;
        }
        const char *tContent = STG.CatFile(tFullPath);
        if (strncmp(tContent, "Error:", 6) == 0) {
          tClient.print(F(COLOR_RED "\r\n"));
          tClient.print(tContent);
          tClient.print(F(COLOR_WHITE "\r\n"));
          return true;
        }
        tClient.print(F(COLOR_WHITE "\r\n"));
        uint32_t tLineCount = 0;
        const char *tTemp = tContent;
        while (*tTemp) {
          if (*tTemp == '\n') ++tLineCount;
          ++tTemp;
        }
        if (strlen(tContent) > 0 && tContent[strlen(tContent)-1] != '\n') ++tLineCount;
        int tWidth = 1;
        if (tLineCount >= 10) tWidth = 2;
        if (tLineCount >= 100) tWidth = 3;
        if (tLineCount >= 1000) tWidth = 4;
        if (tLineCount >= 10000) tWidth = 5;
        if (tLineCount >= 100000) tWidth = 6;
        const char *tLinePtr = tContent;
        const char *tEndPtr = tLinePtr + strlen(tContent);
        uint32_t tLineNum = 1;
        const uint32_t PAGE_LINES = 20;
        uint32_t tPrintedLines = 0;
        while (tLinePtr < tEndPtr) {
          const char *tLineEnd = tLinePtr;
          while (tLineEnd < tEndPtr && *tLineEnd != '\r' && *tLineEnd != '\n') ++tLineEnd;
          char tFmt[8];
          tClient.write(COLOR_YELLOW);
          snprintf(tFmt, sizeof(tFmt), "%%%uu  ", tWidth);
          tClient.printf(tFmt, tLineNum++);
          tClient.write(COLOR_WHITE);
          while (tLinePtr < tLineEnd) tClient.write(*tLinePtr++);
          tClient.print(F("\r\n"));
          tPrintedLines++;
          if (tPrintedLines >= PAGE_LINES) {
            tClient.print(F("\r\n" COLOR_GREEN "--- Press ENTER to continue ---" COLOR_WHITE));
            while (!tClient.available()) vTaskDelay(pdMS_TO_TICKS(5));
            while (tClient.available()) tClient.read();
            tPrintedLines = 0;
            tClient.print(F("\r\n"));
          }
          if (tLinePtr < tEndPtr && *tLinePtr == '\r') ++tLinePtr;
          if (tLinePtr < tEndPtr && *tLinePtr == '\n') ++tLinePtr;
        }
        tClient.print(F("\r\n" COLOR_WHITE));
        return true;
      }
      const char *Help() const override {
        return "cat <filename>                   - show file content";
      }
  };

}
#endif