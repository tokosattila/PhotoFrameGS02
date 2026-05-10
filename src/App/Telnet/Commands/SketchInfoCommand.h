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
        const esp_partition_t *tBoot = esp_ota_get_boot_partition();
        UTL.ByteToReadableSize(tSketchSize, tSketchSizeBuffer, sizeof(tSketchSizeBuffer));
        uint32_t tTotalSize = tRunning ? (uint32_t)tRunning->size : 0;
        UTL.ByteToReadableSize(tTotalSize, tSketchTotalSizeBuffer, sizeof(tSketchTotalSizeBuffer));
        tClient.print(F(COLOR_WHITE "\r\n"));
        snprintf(tText, sizeof(tText), "  Sketch: %s / %s", tSketchSizeBuffer, tSketchTotalSizeBuffer);
        tClient.println(tText);
        if (tRunning) tClient.printf("  Running: %s @ 0x%08x\r\n", tRunning->label, (unsigned)tRunning->address);
        if (tBoot) tClient.printf("  Boot: %s @ 0x%08x\r\n", tBoot->label, (unsigned)tBoot->address);
        tClient.print(F("\r\n"));
        return true;
      }
      const char *Help() const override {
        return "sketchinfo                        - show sketch usage info";
      }
    };

}

#endif