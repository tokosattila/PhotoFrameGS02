#ifndef CONFIG_COMMAND_CLASS
#define CONFIG_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class ConfigCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "config"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        if (!tArguments || tArguments[0] == '\0') {
          tClient.print(F(COLOR_RED "\r\n  Error: Missing key\r\n"));
          tClient.print(F(COLOR_YELLOW "  Usage: config <key> [value]\r\n\r\n" COLOR_WHITE));
          return true;
        }
        const char *tPtr = tArguments;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr == '\0') {
          tClient.print(F(COLOR_RED "\r\n  Error: Missing key\r\n"));
          tClient.print(F(COLOR_YELLOW "  Usage: config <key> [value]\r\n\r\n" COLOR_WHITE));
          return true;
        }
        const char *tKeyStart = tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        size_t tKeyLen = tPtr - tKeyStart;
        char tKey[32] = "";
        if (tKeyLen >= sizeof(tKey)) {
          tClient.print(F(COLOR_RED "\r\n  Error: Key too long\r\n\r\n" COLOR_WHITE));
          return true;
        }
        strncpy(tKey, tKeyStart, tKeyLen);
        tKey[tKeyLen] = '\0';
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr == '\0') {
          if (!CFG.HasConfigKey(tKey)) {
            tClient.print(F(COLOR_RED "\r\n  Error: Key not found\r\n\r\n" COLOR_WHITE));
            return true;
          }
          if (strcasecmp(tKey, "image_file") == 0) {
            const char *tCurrent = CFG.GetConfig(tKey);
            bool tNeedReseed = (!tCurrent || tCurrent[0] == '\0');
            if (!tNeedReseed) {
              SDisplayConfig tDisplayCfg = CFG.Get<SDisplayConfig>();
              char tCurrentPath[128] = "";
              snprintf(tCurrentPath, sizeof(tCurrentPath), "/%s/%s", tDisplayCfg.ImagesDir.c_str(), tCurrent);
              tNeedReseed = !STG.Exists(tCurrentPath);
            }
            if (tNeedReseed) {
              const char *tAutoFile = STG.GetNextFile("");
              if (tAutoFile && tAutoFile[0]) CFG.SaveImageName(tAutoFile);
            }
          }
          const char *tVal = CFG.GetConfig(tKey);
          if (!tVal) tVal = "";
          tClient.printf("\r\n  %s = %s\r\n\r\n" COLOR_WHITE, tKey, tVal);
          return true;
        }
        const char *tValueStart = tPtr;
        bool tInQuote = false;
        if (*tPtr == '"') {
          tInQuote = true;
          ++tPtr;
          tValueStart = tPtr;
        }
        while (*tPtr != '\0') {
          if (tInQuote) {
            if (*tPtr == '"') break;
          } else {
            if (*tPtr == ' ' || *tPtr == '\t') break;
          }
          ++tPtr;
        }
        size_t tValueLen = tPtr - tValueStart;
        char tValue[128] = "";
        if (tValueLen >= sizeof(tValue)) {
          tClient.print(F(COLOR_RED "\r\n  Error: Value too long\r\n\r\n" COLOR_WHITE));
          return true;
        }
        strncpy(tValue, tValueStart, tValueLen);
        tValue[tValueLen] = '\0';
        if (tInQuote && *tPtr == '"') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr != '\0') {
          tClient.print(F(COLOR_RED "\r\n  Error: Too many arguments\r\n"));
          tClient.print(F(COLOR_YELLOW "  Usage: config <key> [value]\r\n\r\n" COLOR_WHITE));
          return true;
        }
        if (CFG.SetConfig(tKey, tValue)) {
          const char *tCFileName = CFG.GetConfig("config_file");
          const char *tCFileContent = CFG.PrepareAllConfigToINI();
          STG.WriteFile(tCFileName, tCFileContent, false);
          tClient.print(F(COLOR_GREEN "\r\n  OK: Value set\r\n\r\n" COLOR_WHITE));
        } else tClient.print(F(COLOR_RED "\r\n  Error: Unknown key or failed to write\r\n\r\n" COLOR_WHITE));
        return true;
      }
      const char *Help() const override { 
        return "config <key> [value]              - get or set config value"; 
      }
    };

}
#endif