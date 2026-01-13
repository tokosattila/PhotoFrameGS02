#ifndef FETCH_COMMAND_CLASS
#define FETCH_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class FetchCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "fetch"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        char tUrl[256];
        char tFilename[128];
        if (!ParseArguments(tArguments, tUrl, sizeof(tUrl), tFilename, sizeof(tFilename), tClient)) return true;
        String tHost, tPath;
        uint16_t tPort;
        bool tIsHttps;
        if (!ParseUrl(tUrl, tHost, tPath, tPort, tIsHttps, tClient)) return true;
        if (!BuildFilePath(tFilename, sizeof(tFilename), tClient)) return true;
        tClient.printf("\r\n  Downloading: %s:%d%s to %s\r\n", tHost.c_str(), tPort, tPath.c_str(), tFilename);
        int tBytesWritten = tIsHttps  ? DownloadHttps(tHost, tPath, tPort, tFilename, tClient) : DownloadHttp(tHost, tPath, tPort, tFilename, tClient);
        return HandleResult(tBytesWritten, tFilename, tClient);
      }     
      const char *Help() const override {
        static char tHelp[128];
        snprintf(tHelp, sizeof(tHelp), "fetch <url> [filename]           - download image (max. %dkB, type: *.jpg, *.jpeg)", kMaxFileSizeKB);
        return tHelp;
      }
    private:
      static constexpr uint16_t kHttpPort = 80;
      static constexpr uint16_t kHttpsPort = 443;
      static constexpr int kMaxFileSizeKB = 400;
      static constexpr int kMaxFileSizeBytes = kMaxFileSizeKB * 1024;
      static constexpr int kHttpsMinHeap = 50 * 1024;
      static constexpr int kTimeoutMs = 10 * 1000;
      static constexpr size_t kBufferSize = 1024;
      static constexpr int kMaxAutoFiles = 99;
      inline void PrintError(WiFiClient &tClient, const char *tMessage) {
        tClient.printf(COLOR_RED "\r\n  Error: %s\r\n" COLOR_YELLOW "  Usage: fetch <url> [filename]\r\n\r\n" COLOR_WHITE, tMessage);
      }
      inline const char *SkipWhitespace(const char *tPtr) {
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        return tPtr;
      }
      inline const char *SkipNonWhitespace(const char *tPtr) {
        while (*tPtr && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        return tPtr;
      }
      bool ParseArguments(const char *tArguments, char *tUrl, size_t tUrlSize, char *tFilename, size_t tFilenameSize, WiFiClient &tClient) {
        if (!tArguments || !*tArguments) {
          PrintError(tClient, "Missing URL");
          return false;
        }
        const char *tPtr = SkipWhitespace(tArguments);
        tPtr = SkipNonWhitespace(tPtr);
        tPtr = SkipWhitespace(tPtr);
        const char *tUrlStart = tPtr;
        tPtr = SkipNonWhitespace(tPtr);
        size_t tUrlLen = tPtr - tUrlStart;
        if (tUrlLen == 0 || tUrlLen >= tUrlSize) {
          PrintError(tClient, "Invalid or too long URL");
          return false;
        }
        memcpy(tUrl, tUrlStart, tUrlLen);
        tUrl[tUrlLen] = '\0';
        tPtr = SkipWhitespace(tPtr);
        if (*tPtr) {
          const char *tFilenameStart = tPtr;
          tPtr = SkipNonWhitespace(tPtr);
          size_t tFilenameLen = tPtr - tFilenameStart;
          if (tFilenameLen >= tFilenameSize) {
            PrintError(tClient, "Filename too long");
            return false;
          }
          if (*SkipWhitespace(tPtr)) {
            PrintError(tClient, "Too many arguments");
            return false;
          }
          memcpy(tFilename, tFilenameStart, tFilenameLen);
          tFilename[tFilenameLen] = '\0';
        } else tFilename[0] = '\0';
        return true;
      }
      bool ParseUrl(const char *tUrl, String &tHost, String &tPath, uint16_t &tPort, bool &tIsHttps, WiFiClient &tClient) {
        const char *tHostStart;
        if (strncmp(tUrl, "https://", 8) == 0) {
          tHostStart = tUrl + 8;
          tPort = kHttpsPort;
          tIsHttps = true;
        } else if (strncmp(tUrl, "http://", 7) == 0) {
          tHostStart = tUrl + 7;
          tPort = kHttpPort;
          tIsHttps = false;
        } else {
          PrintError(tClient, "URL must start with → http:// or https://");
          return false;
        }
        const char *tPathStart = strchr(tHostStart, '/');
        if (tPathStart) {
          tHost = String(tHostStart, tPathStart - tHostStart);
          tPath = String(tPathStart);
        } else {
          tHost = String(tHostStart);
          tPath = "/";
        }
        return true;
      }
      bool BuildFilePath(char *tFilename, size_t tSize, WiFiClient &tClient) {
        SDisplayConfig tCfg = CFG.Get<SDisplayConfig>();
        const char *tImgDir = tCfg.ImagesDir.c_str();
        const char *tImgExt = tCfg.ImageExt.c_str();
        if (!tImgDir || !*tImgDir) tImgDir = "/images";
        if (!tImgExt || !*tImgExt) tImgExt = ".jpg";
        if (!*tFilename) {
          for (int i = 1; i <= kMaxAutoFiles; ++i) {
            snprintf(tFilename, tSize, "%s%s/pic_%02d%s", (tImgDir[0] == '/') ? "" : "/", tImgDir, i, tImgExt);
            File tTestFile = STG.OpenFile(tFilename, "r");
            if (!tTestFile) break;
            tTestFile.close();
          }
        } else if (tFilename[0] != '/') {
          char tTmp[128];
          snprintf(tTmp, sizeof(tTmp), "%s%s/%s", (tImgDir[0] == '/') ? "" : "/", tImgDir, tFilename);
          strncpy(tFilename, tTmp, tSize - 1);
          tFilename[tSize - 1] = '\0';
        }
        const char *tExt = strrchr(tFilename, '.');
        if (!tExt || (strcasecmp(tExt, ".jpg") != 0 && strcasecmp(tExt, ".jpeg") != 0)) {
          PrintError(tClient, "Only → .jpg/.jpeg files allowed");
          return false;
        }
        return true;
      }

      int DownloadHttp(const String &tHost, const String &tPath, uint16_t tPort, const char *tFilename, WiFiClient &tClient) {
        WiFiClient tHttpClient;
        HttpClient tHttp(tHttpClient, tHost, tPort);
        tHttp.get(tPath);
        int tStatusCode = tHttp.responseStatusCode();
        if (tStatusCode != 200) {
          tClient.printf(COLOR_RED "\r\n  Error: HTTP %d\r\n\r\n" COLOR_WHITE, tStatusCode);
          return -1;
        }
        int tContentLength = tHttp.contentLength();
        if (tContentLength > kMaxFileSizeBytes) {
          tClient.printf(COLOR_RED "\r\n  Error: File too large (%d bytes)\r\n\r\n" COLOR_WHITE, tContentLength);
          return -1;
        }
        File tFile = STG.OpenFile(tFilename, "w");
        if (!tFile) {
          PrintError(tClient, "Cannot create file");
          return -1;
        }
        uint8_t tBuffer[kBufferSize];
        int tTotal = 0;
        while (tHttp.available()) {
          int tRead = tHttp.read(tBuffer, sizeof(tBuffer));
          if (tRead > 0) {
            tFile.write(tBuffer, tRead);
            tTotal += tRead;
            if (tTotal > kMaxFileSizeBytes) break;
          }
          vTaskDelay(1);
        }
        tFile.close();
        return tTotal;
      }
      int DownloadHttps(const String &tHost, const String &tPath, uint16_t tPort, const char *tFilename, WiFiClient &tClient) {
        if (ESP.getFreeHeap() < kHttpsMinHeap) {
          PrintError(tClient, "Not enough memory for HTTPS");
          return -1;
        }
        WiFiClientSecure tSecure;
        tSecure.setInsecure();
        tSecure.setHandshakeTimeout(30);
        if (!tSecure.connect(tHost.c_str(), tPort)) {
          char tErr[64] = "";
          tSecure.lastError(tErr, sizeof(tErr));
          tClient.printf(COLOR_RED "\r\n  Error: HTTPS connection failed: %s\r\n\r\n" COLOR_WHITE, tErr);
          return -1;
        }
        tSecure.printf("GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", tPath.c_str(), tHost.c_str());
        unsigned long tDeadline = millis() + kTimeoutMs;
        while (tSecure.connected() && !tSecure.available()) {
          if (millis() > tDeadline) {
            PrintError(tClient, "Response timeout");
            tSecure.stop();
            return -1;
          }
          vTaskDelay(pdMS_TO_TICKS(10));
        }
        String tStatusLine = tSecure.readStringUntil('\n');
        int tIdx = tStatusLine.indexOf(' ');
        int tStatusCode = (tIdx > 0) ? tStatusLine.substring(tIdx + 1, tIdx + 4).toInt() : 0;
        if (tStatusCode != 200) {
          tClient.printf(COLOR_RED "\r\n  Error: HTTP %d\r\n" COLOR_WHITE, tStatusCode);
          tSecure.stop();
          return -1;
        }
        int tContentLength = 0;
        while (tSecure.connected()) {
          String tHeader = tSecure.readStringUntil('\n');
          tHeader.trim();
          if (tHeader.isEmpty()) break;
          if (tHeader.startsWith("Content-Length:")) tContentLength = tHeader.substring(15).toInt();
        }
        if (tContentLength > kMaxFileSizeBytes) {
          tClient.printf(COLOR_RED "\r\n  Error: File too large (%d bytes)\r\n" COLOR_WHITE, tContentLength);
          tSecure.stop();
          return -1;
        }
        File tFile = STG.OpenFile(tFilename, "w");
        if (!tFile) {
          PrintError(tClient, "Cannot create file");
          tSecure.stop();
          return -1;
        }
        uint8_t tBuffer[kBufferSize];
        int tTotal = 0;
        while (tSecure.connected() || tSecure.available()) {
          if (tSecure.available()) {
            int tRead = tSecure.read(tBuffer, sizeof(tBuffer));
            if (tRead > 0) {
              tFile.write(tBuffer, tRead);
              tTotal += tRead;
              if (tTotal > kMaxFileSizeBytes) break;
            }
          }
          vTaskDelay(1);
        }
        tFile.close();
        tSecure.stop();
        return tTotal;
      }
      bool HandleResult(int tBytes, const char *tFilename, WiFiClient &tClient) {
        if (tBytes < 0)  return true;       
        if (tBytes == 0) {
          STG.DeleteFile(tFilename);
          PrintError(tClient, "Downloaded 0 bytes");
          return true;
        }
        if (tBytes > kMaxFileSizeBytes) {
          STG.DeleteFile(tFilename);
          tClient.printf(COLOR_RED "\r\n  Error: File exceeded %dkB limit\r\n" COLOR_WHITE, kMaxFileSizeKB);
          return true;
        }
        char tSizeBuf[16];
        UTL.ByteToReadableSize(tBytes, tSizeBuf, sizeof(tSizeBuf));
        tClient.printf(COLOR_GREEN "\r\n  OK: %s (%s)\r\n\r\n" COLOR_WHITE, tFilename, tSizeBuf);
        return true;
      }
  };

}

#endif