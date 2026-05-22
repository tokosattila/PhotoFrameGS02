#ifndef IMG_INFO_COMMAND_H
#define IMG_INFO_COMMAND_H

#include <App/Telnet/Command.h>

namespace App {

  class ImgInfoCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "imginfo";
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        const char *tPtr = tArguments;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        if (*tPtr != '\0') {
          tClient.print(F(COLOR_RED "\r\n  Usage: imginfo\r\n\r\n" COLOR_WHITE));
          return false;
        }
        SDisplayConfig tDisplayCfg = CFG.Get<SDisplayConfig>();
        STimerConfig tTimerCfg = CFG.Get<STimerConfig>();
        UTL.ReloadConfig();
        char tImagePath[192] = "";
        BuildImagePath(tDisplayCfg, tImagePath, sizeof(tImagePath));
        uint32_t tLastUpdate = CFG.GetImageUpdatedAt();
        uint32_t tNowUtc = static_cast<uint32_t>(time(nullptr));
        if (tNowUtc < 1735689600UL) tNowUtc = static_cast<uint32_t>(RTC.GetEpoch());
        uint32_t tNextUpdate = 0;
        if (tNowUtc > 0) tNextUpdate = tNowUtc + UTL.ComputeWakeDelaySeconds(tTimerCfg);
        char tLastUpdateText[32] = "";
        char tNextUpdateText[32] = "";
        if (tLastUpdate == 0) snprintf(tLastUpdateText, sizeof(tLastUpdateText), "N/A");
        else UTL.EpochToReadableFormat(tLastUpdate, true, tLastUpdateText, sizeof(tLastUpdateText));
        if (tNextUpdate == 0) snprintf(tNextUpdateText, sizeof(tNextUpdateText), "N/A");
        else UTL.EpochToReadableFormat(tNextUpdate, true, tNextUpdateText, sizeof(tNextUpdateText));
        tClient.printf(COLOR_WHITE "\r\n  Current image: %s", tImagePath);
        tClient.printf("\r\n  Last update:   %s", tLastUpdateText);
        tClient.printf("\r\n  Next update:   %s\r\n\r\n", tNextUpdateText);
        return true;
      }
      const char *Help() const override {
        return "imginfo                           - show current image and refresh schedule";
      }
    private:
      static void BuildImagePath(const SDisplayConfig &tDisplayCfg, char *tOutPath, size_t tOutSize) {
        if (!tOutPath || tOutSize == 0) return;
        if (tDisplayCfg.CurrentFile.length() == 0) {
          snprintf(tOutPath, tOutSize, "N/A");
          return;
        }
        const char *tCurrent = tDisplayCfg.CurrentFile.c_str();
        if (tCurrent[0] == '/') {
          snprintf(tOutPath, tOutSize, "%s", tCurrent);
          return;
        }
        const char *tDir = tDisplayCfg.ImagesDir.length() ? tDisplayCfg.ImagesDir.c_str() : IMAGES_DIR;
        if (tDir[0] == '/') snprintf(tOutPath, tOutSize, "%s/%s", tDir, tCurrent);
        else snprintf(tOutPath, tOutSize, "/%s/%s", tDir, tCurrent);
      }
  };

}

#endif
