#ifndef DELETE_COMMAND_CLASS
#define DELETE_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class DeleteCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "delete";
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        char tTarget[16] = "";
        char tRawSpec[256] = "";
        if (!ParseArgs(tArguments, tTarget, sizeof(tTarget), tRawSpec, sizeof(tRawSpec))) {
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
        char tDir[128] = "";
        char tSpec[256] = "";
        if (!UTL.SplitPathAndFile(tRawSpec, tDir, sizeof(tDir), tSpec, sizeof(tSpec))) {
          PrintUsage(tClient);
          return true;
        }
        std::vector<String> tFiles;
        UTL.ResolveFileSpec(tDir, tSpec, tIsSD, tFiles);
        if (tFiles.empty()) {
          tClient.printf(COLOR_YELLOW "\r\n  No files matching: %s\r\n\r\n" COLOR_WHITE, tSpec);
          return true;
        }
        const char *tStorageName = tIsSD ? SDC.GetName() : LFS.GetName();
        tClient.printf(COLOR_YELLOW "\r\n  Delete %u file(s) from %s [%s]:\r\n\r\n" COLOR_WHITE, (unsigned)tFiles.size(), tStorageName, tDir);
        for (const String &tFile : tFiles) {
          tClient.printf("  %s\r\n", tFile.c_str());
        }
        tClient.print(F("\r\n"));
        std::vector<String> tFilesCopy = tFiles;
        String tDirStr(tDir);
        TLN.RequestConfirmation("Delete? (y/n): ",
          [tFilesCopy, tDirStr, tIsSD](bool tConfirmed, WiFiClient &tConfirmClient) {
            if (!tConfirmed) {
              tConfirmClient.print(F("\r\n  Cancelled.\r\n"));
              return;
            }
            uint16_t tDeleted = 0;
            uint16_t tDeleteFailed = 0;
            for (const String &tFile : tFilesCopy) {
              char tFullPath[256] = "";
              snprintf(tFullPath, sizeof(tFullPath), "%s/%s", tDirStr.c_str(), tFile.c_str());
              bool tOk = tIsSD ? SDC.DeleteFile(tFullPath) : LFS.DeleteFile(tFullPath);
              if (tOk) {
                tConfirmClient.printf(COLOR_GREEN "  Deleted: %s\r\n" COLOR_WHITE, tFile.c_str());
                tDeleted++;
              } else {
                tConfirmClient.printf(COLOR_RED "  Failed: %s\r\n" COLOR_WHITE, tFile.c_str());
                tDeleteFailed++;
              }
            }
            tConfirmClient.printf("\r\n  " COLOR_GREEN "Deleted: %u" COLOR_WHITE "  " COLOR_RED "Failed: %u\r\n" COLOR_WHITE, (unsigned)tDeleted, (unsigned)tDeleteFailed);
          });
        return true;
      }
      const char *Help() const override {
        return "delete sd [/path/]<filespec>      " COLOR_YELLOW "- delete from SD Card\r\n  " COLOR_WHITE
               "delete lfs [/path/]<filespec>     " COLOR_YELLOW "- delete from LittleFS" COLOR_WHITE;
      }
    private:
      bool ParseArgs(const char *tInput, char *tTarget, size_t tTargetSize, char *tSpec, size_t tSpecSize) {
        if (!tInput) return false;
        const char *tP = tInput;
        while (*tP && *tP != ' ' && *tP != '\t') ++tP;
        while (*tP == ' ' || *tP == '\t') ++tP;
        const char *tS = tP;
        while (*tP && *tP != ' ' && *tP != '\t') ++tP;
        if (tP == tS) return false;
        size_t tLen = tP - tS;
        if (tLen >= tTargetSize) return false;
        strncpy(tTarget, tS, tLen);
        tTarget[tLen] = '\0';
        while (*tP == ' ' || *tP == '\t') ++tP;
        if (*tP == '\0') return false;
        const char *tEnd = tP + strlen(tP);
        while (tEnd > tP && (*(tEnd - 1) == ' ' || *(tEnd - 1) == '\t')) --tEnd;
        tLen = tEnd - tP;
        if (tLen == 0 || tLen >= tSpecSize) return false;
        strncpy(tSpec, tP, tLen);
        tSpec[tLen] = '\0';
        return true;
      }
      void PrintUsage(WiFiClient &tClient) {
        tClient.print(F(COLOR_YELLOW "\r\n  Usage: delete <target> <filespec>\r\n"));
        tClient.print(F("  Target: sd | sdcard | lfs | littlefs\r\n"));
        tClient.print(F("  Filespec: file.ext | /path/file.ext | *.ext | *pattern*\r\n"));
        tClient.print(F("  Batch:  f1.ext,f2.ext | /path/*.ext\r\n\r\n" COLOR_WHITE));
      }

  };

}

#endif
