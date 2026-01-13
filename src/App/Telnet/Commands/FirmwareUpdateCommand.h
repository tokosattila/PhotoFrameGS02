#ifndef FIRMWARE_UPDATE_COMMAND_CLASS
#define FIRMWARE_UPDATE_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class FirmwareUpdateCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "fwupdate";
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        const char *tFirmwarePath = FIRMWARE_PATH;
        const char *tShaPath = FIRMWARE_SHA_PATH;
        const char *tPtr = tArguments ? tArguments : "";
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        char tSubCommand[16] = "";
        if (*tPtr == '\0') strncpy(tSubCommand, "status", sizeof(tSubCommand) - 1);
        else {
          const char *tStart = tPtr;
          while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
          size_t tLen = (size_t)(tPtr - tStart);
          if (tLen == 0 || tLen >= sizeof(tSubCommand)) {
            tClient.print(F(COLOR_RED "\r\n  Error: Invalid subcommand\r\n"));
            tClient.print(F(COLOR_YELLOW "\r\n  Usage: fwupdate [verify|run]\r\n\r\n" COLOR_WHITE));
            return true;
          }
          strncpy(tSubCommand, tStart, tLen);
          tSubCommand[tLen] = '\0';
          while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
          if (*tPtr != '\0') {
            tClient.print(F(COLOR_RED "\r\n  Error: Too many arguments\r\n"));
            tClient.print(F(COLOR_YELLOW "\r\n  Usage: fwupdate [verify|run]\r\n\r\n" COLOR_WHITE));
            return true;
          }
        }
        {
          Storage_::Guard tStorageGuard;
          if (!STG.IsMounted()) {
            tClient.print(F(COLOR_RED "\r\n  Error: Storage not mounted\r\n\r\n" COLOR_WHITE));
            return true;
          }
          char tFirmwareFullPath[128] = "";
          char tShaFullPath[128] = "";
          strncpy(tFirmwareFullPath, STG.NormalizePath(tFirmwarePath), sizeof(tFirmwareFullPath) - 1);
          strncpy(tShaFullPath, STG.NormalizePath(tShaPath), sizeof(tShaFullPath) - 1);
          tClient.print(F(COLOR_WHITE "\r\n"));
          tClient.printf("  Storage: %s%s\r\n\r\n", STG.GetActiveName(), STG.HasFallback() ? " [fallback]" : "");
          bool tHasFirmware = STG.Exists(tFirmwareFullPath);
          bool tHasSha = STG.Exists(tShaFullPath);
          tClient.printf(COLOR_WHITE "  Firmware: %s %s %s\r\n" COLOR_WHITE, tFirmwareFullPath, tHasFirmware ? COLOR_GREEN : COLOR_RED, tHasFirmware ? "FOUND" : "MISSING");
          tClient.printf(COLOR_WHITE "  SHA256: %s %s %s\r\n" COLOR_WHITE, tShaFullPath, tHasSha ? COLOR_GREEN : COLOR_RED, tHasSha ? "FOUND" : "MISSING");
          if (tHasFirmware) {
            File tFirmwareFile = STG.OpenFile(tFirmwareFullPath, FILE_READ);
            if (tFirmwareFile) {
              char tSizeBuffer[16] = "";
              uint32_t tSize = (uint32_t)tFirmwareFile.size();
              UTL.ByteToReadableSize(tSize, tSizeBuffer, sizeof(tSizeBuffer));
              tClient.printf(COLOR_WHITE "\r\n  Firmware size: %s (%u bytes)\r\n", tSizeBuffer, (unsigned)tSize);
              tFirmwareFile.close();
            }
          }
          if (tHasSha) {
            File tShaFile = STG.OpenFile(tShaFullPath, FILE_READ);
            if (tShaFile) {
              char tSizeBuffer[16] = "";
              uint32_t tSize = (uint32_t)tShaFile.size();
              UTL.ByteToReadableSize(tSize, tSizeBuffer, sizeof(tSizeBuffer));
              tClient.printf(COLOR_WHITE "  SHA256 size: %s (%u bytes)\r\n", tSizeBuffer, (unsigned)tSize);
              tShaFile.close();
            }
          }
        }
        if (strcasecmp(tSubCommand, "status") == 0) {
          tClient.print(F(COLOR_YELLOW "  Usage: fwupdate [verify|run]" COLOR_WHITE "\r\n\r\n"));
          return true;
        }
        if (!FWU.Init()) {
          tClient.printf("\r\n" COLOR_RED "  Error: %s" COLOR_WHITE "\r\n\r\n", FWU.GetLastError() ? FWU.GetLastError() : "init failed");
          return true;
        }
        if (!FWU.UpdateAvailable()) {
          const char *tError = FWU.GetLastError();
          if (tError) tClient.printf(COLOR_RED "  Error: %s" COLOR_WHITE "\r\n\r\n", tError);
          else tClient.print(F(COLOR_RED "  Error: No firmware update found" COLOR_WHITE "\r\n\r\n"));
          return true;
        }
        if (strcasecmp(tSubCommand, "verify") == 0) {
          if (!FWU.VerifySha256()) {
            tClient.printf("\r\n" COLOR_RED "  Error: %s" COLOR_WHITE "\r\n\r\n", FWU.GetLastError() ? FWU.GetLastError() : "sha256 verification failed");
            return true;
          }
          tClient.print(F("\r\n" COLOR_GREEN "  OK: firmware.bin SHA256 verified" COLOR_WHITE "\r\n\r\n"));
          return true;
        }
        if (strcasecmp(tSubCommand, "run") != 0) {
          tClient.print(F(COLOR_RED "\r\n  Error: Unknown subcommand\r\n"));
          tClient.print(F(COLOR_YELLOW "  Usage: fwupdate [verify|run]\r\n\r\n" COLOR_WHITE));
          return true;
        }
        if (!FWU.VerifySha256()) {
          tClient.printf("\r\n" COLOR_RED "  Error: %s" COLOR_WHITE "\r\n\r\n", FWU.GetLastError() ? FWU.GetLastError() : "sha256 verification failed");
          return true;
        }
        tClient.print(F("\r\n" COLOR_YELLOW "  Warning:" COLOR_WHITE " this will write flash and reboot\r\n"));
        tClient.print(F("\r\n  Do not power off during update\r\n\r\n"));
        TLN.RequestConfirmation("Proceed with firmware update? (y/n): ", [](bool tConfirmed, WiFiClient &tConfirmClient) {
          if (!tConfirmed) {
            tConfirmClient.print(F("\r\n  Cancelled\r\n"));
            return;
          }
          tConfirmClient.print(F("\r\n" COLOR_YELLOW "  Updating..." COLOR_WHITE "\r\n"));
          if (!FWU.PerformUpdate(&tConfirmClient)) {
            tConfirmClient.printf("\r\n" COLOR_RED "  Error: %s" COLOR_WHITE "\r\n", FWU.GetLastError() ? FWU.GetLastError() : "update failed");
            return;
          }
          tConfirmClient.print(F("\r\n" COLOR_GREEN "  Update successful." COLOR_WHITE "\r\n"));
          tConfirmClient.print(F(COLOR_YELLOW "  Rebooting in 3 seconds..." COLOR_WHITE "\r\n"));
          tConfirmClient.flush();
          vTaskDelay((3 * DELAY_ONE_SEC_MS) / portTICK_PERIOD_MS);
          esp_restart();
        });
        return true;
      }
      const char *Help() const override {
         return "fwupdate                         " COLOR_YELLOW "- show update status\r\n  " COLOR_WHITE
                "fwupdate verify                  " COLOR_YELLOW "- verify firmware.bin/firmware.sha256\r\n  " COLOR_WHITE
                "fwupdate run                     " COLOR_YELLOW "- perform update (asks y/n)" COLOR_WHITE;
      }
  };

}

#endif
