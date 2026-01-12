#ifndef SKETCHINFO_COMMAND_CLASS
#define SKETCHINFO_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class SketchInfoCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "sketchinfo";
      }
      bool Execute(const char *tArguments, WiFiClient& tClient) override {
        char tText[30] = "";
        char tSketchSizeBuffer[16] = "";
        char tSketchTotalSizeBuffer[16] = "";
        uint32_t tSketchSize = ESP.getSketchSize();
        const esp_partition_t *tRunning = esp_ota_get_running_partition();
        UTL.ByteToReadableSize(tSketchSize, tSketchSizeBuffer, sizeof(tSketchSizeBuffer));
        UTL.ByteToReadableSize(tRunning->size, tSketchTotalSizeBuffer, sizeof(tSketchTotalSizeBuffer));
        tClient.print(F(COLOR_WHITE "\r\n"));
        snprintf(tText, sizeof(tText), "  Sketch: %s / %s", tSketchSizeBuffer, tSketchTotalSizeBuffer);
        tClient.println(tText);
        tClient.print(F("\r\n"));
        return true;
      }
      const char *Help() const override {
        return "sketchinfo                       - show sketch usage info";
      }
    };

}

#endif