#ifndef NVSINFO_COMMAND_CLASS
#define NVSINFO_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class NvsInfoCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "nvsinfo";
      }
      bool Execute(const char *tArguments, WiFiClient& tClient) override {
        nvs_stats_t tStats{};
        if (nvs_get_stats(nullptr, &tStats) != ESP_OK) {
          tClient.print(F(COLOR_RED "\r\n  Error: Cannot read NVS statistics\r\n\r\n" COLOR_WHITE));
          return true;
        }
        if (!mCFG.begin("cfg", true)) {
          tClient.print(F(COLOR_RED "\r\n  Error: Cannot open 'cfg' namespace\r\n\r\n" COLOR_WHITE));
          return true;
        }
        size_t tUsedInCfg = 0;
        nvs_iterator_t tIt = nvs_entry_find("nvs", "cfg", NVS_TYPE_ANY);
        while (tIt != nullptr) {
          nvs_entry_info_t tInfo;
          nvs_entry_info(tIt, &tInfo);
          ++tUsedInCfg;
          tIt = nvs_entry_next(tIt);
        }
        nvs_release_iterator(tIt);
        mCFG.end();
        tClient.print(F(COLOR_WHITE "\r\n"));
        char tBuf[128] = "";
        snprintf(tBuf, sizeof(tBuf), "  Config entries: %u / %u", tUsedInCfg, tStats.total_entries);
        tClient.println(tBuf);
        snprintf(tBuf, sizeof(tBuf), "  All entries: %u / %u", tStats.used_entries, tStats.total_entries);
        tClient.println(tBuf);
        uint32_t tUsedBytes = tUsedInCfg * 32;
        uint32_t tTotalBytes = tStats.total_entries * 32;
        char tUsedBytesBuffer[16] = "";
        char tTotalBytesBuffer[16] = "";
        UTL.ByteToReadableSize(tUsedBytes, tUsedBytesBuffer, sizeof(tUsedBytesBuffer));
        UTL.ByteToReadableSize(tTotalBytes, tTotalBytesBuffer, sizeof(tTotalBytesBuffer));
        snprintf(tBuf, sizeof(tBuf), "  Nvs usage: ~%s / ~%s", tUsedBytesBuffer, tTotalBytesBuffer);
        tClient.println(tBuf);
        tClient.print(F("\r\n"));
        return true;
      }
      const char *Help() const override {
        return "nvsinfo                           - show NVS usage info";
      }
    private:
      Preferences mCFG;
  };

}
#endif