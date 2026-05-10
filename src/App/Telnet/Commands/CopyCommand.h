#ifndef COPY_COMMAND_CLASS
#define COPY_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class CopyCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "copy";
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        char tFrom[16] = "";
        char tTo[16] = "";
        char tRawSpec[256] = "";
        if (!ParseArgs(tArguments, tFrom, sizeof(tFrom), tTo, sizeof(tTo), tRawSpec, sizeof(tRawSpec))) {
          PrintUsage(tClient);
          return true;
        }
        if (!UTL.IsValidTarget(tFrom) || !UTL.IsValidTarget(tTo)) {
          tClient.print(F(COLOR_RED "\r\n  Error: Invalid storage target\r\n" COLOR_WHITE));
          PrintUsage(tClient);
          return true;
        }
        if (UTL.IsSameTarget(tFrom, tTo)) {
          tClient.print(F(COLOR_RED "\r\n  Error: Source and target are the same\r\n\r\n" COLOR_WHITE));
          return true;
        }
        bool tSrcSD = UTL.IsSD(tFrom);
        if (tSrcSD && !SDC.IsMounted()) {
          tClient.print(F(COLOR_RED "\r\n  SD Card is not mounted.\r\n\r\n" COLOR_WHITE));
          return true;
        }
        if (!tSrcSD && !LFS.IsMounted()) {
          tClient.print(F(COLOR_RED "\r\n  LittleFS is not mounted.\r\n\r\n" COLOR_WHITE));
          return true;
        }
        bool tDstSD = !tSrcSD;
        if (tDstSD && !SDC.IsMounted()) {
          tClient.print(F(COLOR_RED "\r\n  SD Card is not mounted.\r\n\r\n" COLOR_WHITE));
          return true;
        }
        if (!tDstSD && !LFS.IsMounted()) {
          tClient.print(F(COLOR_RED "\r\n  LittleFS is not mounted.\r\n\r\n" COLOR_WHITE));
          return true;
        }
        char tDir[128] = "";
        char tSpec[256] = "";
        if (!UTL.SplitPathAndFile(tRawSpec, tDir, sizeof(tDir), tSpec, sizeof(tSpec))) {
          PrintUsage(tClient);
          return true;
        }
        if (tDstSD) SDC.CreateDir(tDir);
        else LFS.CreateDir(tDir);
        const char *tSrcName = tSrcSD ? SDC.GetName() : LFS.GetName();
        const char *tDstName = tDstSD ? SDC.GetName() : LFS.GetName();
        tClient.printf(COLOR_GREEN "\r\n  Copy %s -> %s [%s]:\r\n\r\n" COLOR_WHITE, tSrcName, tDstName, tDir);
        std::vector<String> tFiles;
        UTL.ResolveFileSpec(tDir, tSpec, tSrcSD, tFiles);
        if (tFiles.empty()) {
          tClient.printf(COLOR_YELLOW "  No files matching: %s\r\n\r\n" COLOR_WHITE, tSpec);
          return true;
        }
        uint16_t tCopied = 0;
        uint16_t tFailed = 0;
        for (const String &tFile : tFiles) {
          if (CopyOneFile(tDir, tFile.c_str(), tSrcSD, tClient)) tCopied++;
          else tFailed++;
        }
        tClient.printf("\r\n  " COLOR_GREEN "Copied: %u" COLOR_WHITE "  " COLOR_RED "Failed: %u\r\n\r\n" COLOR_WHITE, (unsigned)tCopied, (unsigned)tFailed);
        return true;
      }
      const char *Help() const override {
        return "copy sd lfs [/path/]<filespec>    " COLOR_YELLOW "- copy SD Card -> LittleFS\r\n  " COLOR_WHITE
               "copy lfs sd [/path/]<filespec>    " COLOR_YELLOW "- copy LittleFS -> SD Card" COLOR_WHITE;             
      }
    private:
      bool ParseArgs(const char *tInput, char *tFrom, size_t tFromSize, char *tTo, size_t tToSize, char *tSpec, size_t tSpecSize) {
        if (!tInput) return false;
        const char *tP = tInput;
        while (*tP && *tP != ' ' && *tP != '\t') ++tP;
        while (*tP == ' ' || *tP == '\t') ++tP;
        const char *tS = tP;
        while (*tP && *tP != ' ' && *tP != '\t') ++tP;
        if (tP == tS) return false;
        size_t tLen = tP - tS;
        if (tLen >= tFromSize) return false;
        strncpy(tFrom, tS, tLen);
        tFrom[tLen] = '\0';
        while (*tP == ' ' || *tP == '\t') ++tP;
        tS = tP;
        while (*tP && *tP != ' ' && *tP != '\t') ++tP;
        if (tP == tS) return false;
        tLen = tP - tS;
        if (tLen >= tToSize) return false;
        strncpy(tTo, tS, tLen);
        tTo[tLen] = '\0';
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
      bool CopyOneFile(const char *tDir, const char *tFileName, bool tSrcSD, WiFiClient &tClient) {
        char tPath[256] = "";
        snprintf(tPath, sizeof(tPath), "%s/%s", tDir, tFileName);
        File tSrc = tSrcSD ? SDC.OpenFile(tPath, FILE_READ) : LFS.OpenFile(tPath, FILE_READ);
        if (!tSrc) {
          tClient.printf(COLOR_RED "  Error: %s not found\r\n" COLOR_WHITE, tFileName);
          return false;
        }
        char tDstPath[256] = "";
        strncpy(tDstPath, tPath, sizeof(tDstPath) - 1);
        bool tDstExists = tSrcSD ? LFS.Exists(tDstPath) : SDC.Exists(tDstPath);
        if (tDstExists) {
          tClient.printf(COLOR_YELLOW "  %s already exists. (o)verwrite / (r)ename / (s)kip? " COLOR_WHITE, tFileName);
          char tChoice = WaitForChar(tClient);
          tClient.printf("%c\r\n", tChoice);
          if (tChoice == 's' || tChoice == 'S') {
            tClient.printf(COLOR_YELLOW "  Skipped: %s\r\n" COLOR_WHITE, tFileName);
            tSrc.close();
            return true;
          }
          if (tChoice == 'r' || tChoice == 'R') {
            if (!GenerateUniqueName(tDir, tFileName, tDstPath, sizeof(tDstPath), !tSrcSD)) {
              tClient.printf(COLOR_RED "  Error: Cannot generate unique name for %s\r\n" COLOR_WHITE, tFileName);
              tSrc.close();
              return false;
            }  
          }
        }
        File tDst = tSrcSD ? LFS.OpenFile(tDstPath, FILE_WRITE, true) : SDC.OpenFile(tDstPath, FILE_WRITE, true);
        if (!tDst) {
          tSrc.close();
          tClient.printf(COLOR_RED "  Error: Cannot create %s\r\n" COLOR_WHITE, tDstPath);
          return false;
        }
        size_t tTotal = tSrc.size();
        size_t tWritten = 0;
        uint8_t tBuffer[1024];
        bool tOk = true;
        while (tSrc.available()) {
          size_t tRead = tSrc.read(tBuffer, sizeof(tBuffer));
          if (tRead == 0) break;
          size_t tW = tDst.write(tBuffer, tRead);
          if (tW != tRead) {
            tOk = false;
            break;
          }
          tWritten += tW;
        }
        tDst.flush();
        tSrc.close();
        tDst.close();
        if (tOk && tWritten == tTotal) {
          char tSize[16] = "";
          UTL.ByteToReadableSize(tTotal, tSize, sizeof(tSize));
          const char *tDstName = strrchr(tDstPath, '/');
          tDstName = tDstName ? tDstName + 1 : tDstPath;
          tClient.printf(COLOR_GREEN "  OK: %s [%s]\r\n" COLOR_WHITE, tDstName, tSize);
          return true;
        }
        tClient.printf(COLOR_RED "  Error: %s (%u/%u bytes)\r\n" COLOR_WHITE, tFileName, (unsigned)tWritten, (unsigned)tTotal);
        return false;
      }
      char WaitForChar(WiFiClient &tClient) {
        unsigned long tStart = millis();
        while (tClient.connected() && (millis() - tStart) < 30000) {
          if (tClient.available()) {
            char tC = tClient.read();
            while (tClient.available()) tClient.read();
            return tC;
          }
          vTaskDelay(50 / portTICK_PERIOD_MS);
        }
        return 's';
      }
      bool GenerateUniqueName(const char *tDir, const char *tFileName, char *tOutPath, size_t tOutSize, bool tIsSD) {
        const char *tDot = strrchr(tFileName, '.');
        char tBase[128] = "";
        char tExt[32] = "";
        if (tDot) {
          size_t tBaseLen = tDot - tFileName;
          if (tBaseLen >= sizeof(tBase)) tBaseLen = sizeof(tBase) - 1;
          strncpy(tBase, tFileName, tBaseLen);
          tBase[tBaseLen] = '\0';
          strncpy(tExt, tDot, sizeof(tExt) - 1);
          tExt[sizeof(tExt) - 1] = '\0';
        } else {
          strncpy(tBase, tFileName, sizeof(tBase) - 1);
          tBase[sizeof(tBase) - 1] = '\0';
        }
        for (uint16_t tIdx = 1; tIdx < 1000; ++tIdx) {
          snprintf(tOutPath, tOutSize, "%s/%s_%u%s", tDir, tBase, (unsigned)tIdx, tExt);
          bool tExists = tIsSD ? SDC.Exists(tOutPath) : LFS.Exists(tOutPath);
          if (!tExists) return true;
        }
        return false;
      }
      void PrintUsage(WiFiClient &tClient) {
        tClient.print(F(COLOR_YELLOW "\r\n  Usage: copy <from> <to> <filespec>\r\n"));
        tClient.print(F("  From/To: sd | sdcard | lfs | littlefs\r\n"));
        tClient.print(F("  Filespec: file.ext | /path/file.ext | *.ext | *pattern*\r\n"));
        tClient.print(F("  Batch:  f1.ext,f2.ext | /path/*.ext\r\n\r\n" COLOR_WHITE));
      }

  };

}

#endif
