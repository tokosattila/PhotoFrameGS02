#ifndef FORMAT_COMMAND_CLASS
#define FORMAT_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class FormatCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "format";
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        char tTarget[16] = "";
        if (!ParseTarget(tArguments, tTarget, sizeof(tTarget))) {
          PrintUsage(tClient);
          return true;
        }
        if (!UTL.IsValidTarget(tTarget)) {
          tClient.print(F(COLOR_RED "\r\n  Error: Invalid storage target\r\n" COLOR_WHITE));
          PrintUsage(tClient);
          return true;
        }
        bool tIsSD = UTL.IsSD(tTarget);
        if (tIsSD && !SDC.IsMounted()) {
          tClient.print(F(COLOR_RED "\r\n  SD Card is not mounted.\r\n\r\n" COLOR_WHITE));
          return true;
        }
        if (!tIsSD && !LFS.IsMounted()) {
          tClient.print(F(COLOR_RED "\r\n  LittleFS is not mounted.\r\n\r\n" COLOR_WHITE));
          return true;
        }
        const char *tStorageName = tIsSD ? SDC.GetName() : LFS.GetName();
        tClient.printf(COLOR_YELLOW "\r\n  Warning:" COLOR_WHITE " this will erase all files and directories on %s\r\n", tStorageName);
        tClient.print(F("\r\n  This operation cannot be undone.\r\n\r\n"));
        char tPrompt[64] = "";
        snprintf(tPrompt, sizeof(tPrompt), "Format %s? (y/n): ", tStorageName);
        TLN.RequestConfirmation(tPrompt, [this, tIsSD](bool tConfirmed, WiFiClient &tConfirmClient) {
          if (!tConfirmed) {
            tConfirmClient.print(F("\r\n  Cancelled\r\n\r\n"));
            return;
          }
          bool tOk = tIsSD ? FormatSD() : FormatLFS();
          if (!tOk) tConfirmClient.print(F(COLOR_RED "\r\n  Error: format failed\r\n\r\n" COLOR_WHITE));
          else tConfirmClient.print(F(COLOR_GREEN "\r\n  Format complete\r\n\r\n" COLOR_WHITE));
        });
        return true;
      }
      const char *Help() const override {
        return "format sd|sdcard                  " COLOR_YELLOW "- erase all content on SD Card (asks y/n)\r\n  " COLOR_WHITE
               "format lfs|littlefs               " COLOR_YELLOW "- erase all content on LittleFS (asks y/n)" COLOR_WHITE;
      }
    private:
      bool ParseTarget(const char *tInput, char *tTarget, size_t tTargetSize) {
        if (!tInput || !tTarget || tTargetSize == 0) return false;
        const char *tPtr = tInput;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr == '\0') return false;
        const char *tStart = tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        size_t tLen = (size_t)(tPtr - tStart);
        if (tLen == 0 || tLen >= tTargetSize) return false;
        strncpy(tTarget, tStart, tLen);
        tTarget[tLen] = '\0';
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr != '\0') return false;
        return true;
      }
      void PrintUsage(WiFiClient &tClient) {
        tClient.print(F(COLOR_YELLOW "\r\n  Usage: format <target>\r\n" COLOR_WHITE));
        tClient.print(F(COLOR_WHITE "  target: sd|sdcard|lfs|littlefs\r\n\r\n"));
      }
      bool FormatSD() {
        return SDC.Format();
      }
      bool FormatLFS() {
        return LFS.Format();
      }
  };

}

#endif