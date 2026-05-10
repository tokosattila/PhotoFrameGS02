#ifndef BOOTPARTITION_COMMAND_CLASS
#define BOOTPARTITION_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class BootPartitionCommand_ : public Command_ {
    public:
      const char *GetName() const override {
        return "bootpart";
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        const char *tPtr = tArguments ? tArguments : "";
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
        while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
        char tArg[16] = "";
        if (*tPtr == '\0') strncpy(tArg, "status", sizeof(tArg) - 1);
        else {
          const char *tStart = tPtr;
          while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
          size_t tLen = (size_t)(tPtr - tStart);
          if (tLen == 0 || tLen >= sizeof(tArg)) {
            tClient.print(F(COLOR_RED "\r\n  Error: Invalid argument\r\n"));
            tClient.print(F(COLOR_YELLOW "\r\n  Usage: bootpart [status|ota0|ota1]\r\n\r\n" COLOR_WHITE));
            return true;
          }
          strncpy(tArg, tStart, tLen);
          tArg[tLen] = '\0';
          while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
          if (*tPtr != '\0') {
            tClient.print(F(COLOR_RED "\r\n  Error: Too many arguments\r\n"));
            tClient.print(F(COLOR_YELLOW "\r\n  Usage: bootpart [status|ota0|ota1]\r\n\r\n" COLOR_WHITE));
            return true;
          }
        }
        const esp_partition_t *tRunning = esp_ota_get_running_partition();
        const esp_partition_t *tBoot = esp_ota_get_boot_partition();
        tClient.print(F(COLOR_WHITE "\r\n"));
        if (tRunning) tClient.printf("  Running: %s @ 0x%08x\r\n", tRunning->label, (unsigned)tRunning->address);
        if (tBoot) tClient.printf("  Boot: %s @ 0x%08x\r\n", tBoot->label, (unsigned)tBoot->address);
        if (strcasecmp(tArg, "status") == 0) {
          tClient.print(F("\r\n" COLOR_YELLOW "  Usage: bootpart [status|ota0|ota1]" COLOR_WHITE "\r\n\r\n"));
          return true;
        }
        const esp_partition_t *tTarget = nullptr;
        if (strcasecmp(tArg, "ota0") == 0) tTarget = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, "ota_0");
        else if (strcasecmp(tArg, "ota1") == 0) tTarget = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, "ota_1");
        else {
          tClient.print(F(COLOR_RED "  Error: Unknown argument\r\n"));
          tClient.print(F(COLOR_YELLOW "\r\n  Usage: bootpart [status|ota0|ota1]\r\n\r\n" COLOR_WHITE));
          return true;
        }
        if (!tTarget) {
          tClient.print(F(COLOR_RED "  Error: Target partition not found (requires ota_0/ota_1)\r\n\r\n" COLOR_WHITE));
          return true;
        }
        esp_err_t tErr = esp_ota_set_boot_partition(tTarget);
        if (tErr != ESP_OK) {
          tClient.printf(COLOR_RED "  Error: set boot partition failed (err=%d)\r\n\r\n" COLOR_WHITE, (int)tErr);
          return true;
        }
        const esp_partition_t *tBootAfter = esp_ota_get_boot_partition();
        if (tBootAfter) tClient.printf(COLOR_GREEN "\r\n  OK: Boot set to %s @ 0x%08x\r\n" COLOR_WHITE, tBootAfter->label, (unsigned)tBootAfter->address);
        tClient.print(F(COLOR_YELLOW "\r\n  Run: reboot\r\n\r\n" COLOR_WHITE));
        return true;  
      }
      const char *Help() const override {
        return "bootpart status|ota0|ota1         - show/set boot OTA slot";
      }
  };

}

#endif
