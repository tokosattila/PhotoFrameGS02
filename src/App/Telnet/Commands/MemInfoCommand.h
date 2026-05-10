#ifndef MEMINFO_COMMAND_CLASS
#define MEMINFO_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class MemInfoCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "meminfo";
      }
      bool Execute(const char *tArguments, WiFiClient& tClient) override {
        char tText[128] = "";
        char tUsedBuf[16] = "";
        char tTotalBuf[16] = "";
        tClient.print(F(COLOR_WHITE "\r\n"));
        uint32_t tDramTotal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        uint32_t tDramFree = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        uint32_t tDramUsed = tDramTotal - tDramFree;
        UTL.ByteToReadableSize(tDramUsed, tUsedBuf, sizeof(tUsedBuf));
        UTL.ByteToReadableSize(tDramTotal, tTotalBuf, sizeof(tTotalBuf));
        snprintf(tText, sizeof(tText), "  DRAM: %s / %s", tUsedBuf, tTotalBuf);
        tClient.println(tText);
        uint32_t tIramTotal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_EXEC);
        uint32_t tIramFree  = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_EXEC);
        uint32_t tIramUsed  = tIramTotal - tIramFree;
        UTL.ByteToReadableSize(tIramUsed, tUsedBuf, sizeof(tUsedBuf));
        UTL.ByteToReadableSize(tIramTotal, tTotalBuf, sizeof(tTotalBuf));
        snprintf(tText, sizeof(tText), "  IRAM: %s / %s", tUsedBuf, tTotalBuf);
        tClient.println(tText);
        uint32_t tPsramTotal = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        if (tPsramTotal > 0) {
          uint32_t tPsramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
          uint32_t tPsramUsed = tPsramTotal - tPsramFree;
          UTL.ByteToReadableSize(tPsramUsed, tUsedBuf, sizeof(tUsedBuf));
          UTL.ByteToReadableSize(tPsramTotal, tTotalBuf, sizeof(tTotalBuf));
          snprintf(tText, sizeof(tText), "  PSRAM: %s / %s", tUsedBuf, tTotalBuf);
          tClient.println(tText);
        }
        uint32_t tFreeHeap = ESP.getFreeHeap();
        uint32_t tLargest = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        UTL.ByteToReadableSize(tFreeHeap, tUsedBuf, sizeof(tUsedBuf));
        snprintf(tText, sizeof(tText), "  FREE HEAP: %s", tUsedBuf);
        tClient.println(tText);
        tClient.print(F("\r\n"));
        return true;
      }
      const char *Help() const override {
        return "meminfo                           - show memory usage info";
      }
    };

}
#endif