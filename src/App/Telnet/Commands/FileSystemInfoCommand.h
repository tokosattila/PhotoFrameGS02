#ifndef FILESYSTEMINFO_COMMAND_CLASS
#define FILESYSTEMINFO_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class FileSystemInfoCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "fsinfo";
      }
      bool Execute(const char *tArguments, WiFiClient& tClient) override {
        char tText[64] = "";
        char tUsedBuffer[16] = "";
        char tTotalBuffer[16] = "";
        tClient.print(F(COLOR_WHITE "\r\n"));
        
        // Show active storage info
        UTL.ByteToReadableSize(STG.UsedBytes(), tUsedBuffer, sizeof(tUsedBuffer));
        UTL.ByteToReadableSize(STG.TotalBytes(), tTotalBuffer, sizeof(tTotalBuffer));
        snprintf(tText, sizeof(tText), "  %s (active): %s / %s%s", 
                 STG.GetActiveName(), tUsedBuffer, tTotalBuffer,
                 STG.HasFallback() ? " [fallback]" : "");
        tClient.println(tText);
        
        // Show LittleFS info if available and not active
        if (LFS.IsMounted() && !STG.IsLittleFS()) {
          UTL.ByteToReadableSize(LFS.UsedBytes(), tUsedBuffer, sizeof(tUsedBuffer));
          UTL.ByteToReadableSize(LFS.TotalBytes(), tTotalBuffer, sizeof(tTotalBuffer));
          snprintf(tText, sizeof(tText), "  LittleFS: %s / %s", tUsedBuffer, tTotalBuffer);
          tClient.println(tText);
        }
        
        // Show SDCard info if available and not active
        if (SDC.IsMounted() && !STG.IsSDCard()) {
          UTL.ByteToReadableSize(SDC.UsedBytes(), tUsedBuffer, sizeof(tUsedBuffer));
          UTL.ByteToReadableSize(SDC.TotalBytes(), tTotalBuffer, sizeof(tTotalBuffer));
          snprintf(tText, sizeof(tText), "  SDCard (%s): %s / %s", SDC.CardTypeName(), tUsedBuffer, tTotalBuffer);
          tClient.println(tText);
        }
        
        tClient.print(F("\r\n"));
        return true;
      }
      const char *Help() const override {
        return "fsinfo                           - show filesystem usage info";
      }
    };

}
#endif