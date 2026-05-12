#ifndef GLOBAL_H
#define GLOBAL_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <driver/gpio.h>
#include <driver/touch_pad.h>
#include <esp_adc_cal.h>
#include <esp_bt.h>
#include <esp_chip_info.h>
#include <esp_heap_caps.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_sleep.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <soc/rtc_cntl_reg.h>
#include <soc/soc.h>
#include <ctype.h>
#include <stdio.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <atomic>
#include <functional>
#include <vector>
#include <mbedtls/sha256.h>
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#include <USB.h>
#include <Update.h>
#include <Wire.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <ArduinoHttpClient.h>
#include <epd_driver.h>
#include <i2s_data_bus.h>
#include <JPEGDEC.h>
#include <SimpleFTPServer.h>

namespace App {

  #define PRODUCTION true
  
  #define DEFINE_TAG(tTag) static constexpr const char *TAG = tTag

  #if PRODUCTION
    #define xLOG_S
    #define xLOG_FLUSH()
    #define xLOG_B(...)
    #define xLOG_PR(...)
    #define xLOG_PL(...)
    #define xLOG_PR_(...)
    #define xLOG_PL_(...)
    #define xLOG_PF(...)
    #define xLOG(...)
    #define xLOG_E()
  #else
    #define xLOG_S USBSerial
    #define xLOG_FLUSH() USBSerial.flush()
    #define xLOG_B(...) USBSerial.begin(__VA_ARGS__)
    #define xLOG_PR(...) USBSerial.print(__VA_ARGS__)
    #define xLOG_PL(...) USBSerial.println(__VA_ARGS__)
    #define xLOG_PR_(...) USBSerial.print(F(__VA_ARGS__))
    #define xLOG_PL_(...) USBSerial.println(F(__VA_ARGS__))
    #define xLOG_PF(...) USBSerial.printf(__VA_ARGS__)
    #define xLOG(...) do { \
      constexpr const char *tTag = TAG ? TAG : "[APP]"; \
      char tBuffer[256] = ""; \
      snprintf(tBuffer, sizeof(tBuffer), __VA_ARGS__); \
      USBSerial.printf("[%s] %s\n", tTag, tBuffer); \
    } while(0)
    #define xLOG_E() USBSerial.end()
  #endif

  using FDefaultCallback = std::function<void()>;

  using FConnectionCallback = FDefaultCallback;

  enum class ECPUFrequency : uint8_t {
    F80MHz = 80,
    F160MHz = 160,
    F240MHz = 240
  };

  enum class EDevicePins : uint8_t {
    Btn1 = 21U,
    Btn2 = 48U,
    PiezoPin = 47U,
    BatPin = 14U,
    SDMisoPin = 16U,
    SDMosiPin = 15U,
    SDSckPin = 11U,
    SDCsPin = 42U,
    RTCSdaPin = 18U,
    RTCSclPin = 17U
  };

  enum class EFileSystemType : uint8_t {
    LittleFS = 1,
    SDCard
  };

  enum class ETimerWakeUp : uint8_t {
    Minutes = 1,
    Hourly,
    HalfDay,
    Daily,
    Weekly,
    Monthly
  };

  template<typename T>
  class AutoGuard {
    public:
      AutoGuard() { T::Lock(); }
      ~AutoGuard() { T::Unlock(); }
      AutoGuard(const AutoGuard&) = delete;
      AutoGuard& operator=(const AutoGuard&) = delete;
  };

  class Percentage {
    public:
      constexpr explicit Percentage(uint8_t tValue = 0) : mValue(tValue) {}
      constexpr uint8_t Get() const { return mValue; }
      constexpr operator uint8_t() const { return mValue; }
      bool operator==(const Percentage& tOther) const { return mValue == tOther.mValue; }
      bool operator!=(const Percentage& tOther) const { return mValue != tOther.mValue; }
    private:
      uint8_t mValue;
  };

  class Port {
    public:
      constexpr explicit Port(uint16_t tValue = 0) : mValue(tValue == 0 ? 1 : tValue) {}
      constexpr uint16_t Get() const { return mValue; }
      constexpr operator uint16_t() const { return mValue; }
      bool IsValid() const { return mValue > 0; }
      bool operator==(const Port& tOther) const { return mValue == tOther.mValue; }
      bool operator!=(const Port& tOther) const { return mValue != tOther.mValue; }
    private:
      uint16_t mValue;
  };

  struct SDirEntry {
    char Name[128] = "";
    bool IsDir = false;
    size_t Size = 0;
  };  

  struct SToneStep {
    constexpr SToneStep(uint16_t tFrequencyHz = 0, uint16_t tDurationMs = 0, uint16_t tPauseMs = 0, uint8_t tDutyPct = 50) : FrequencyHz(tFrequencyHz), DurationMs(tDurationMs), PauseMs(tPauseMs), DutyPct(tDutyPct) {}
    uint16_t FrequencyHz;
    uint16_t DurationMs;
    uint16_t PauseMs;
    uint8_t DutyPct;
  };

  struct SDeviceConfig {
    String Name;
    String Version;
    String ConfigFile;
    uint8_t BatteryPin = 0;
    uint8_t ResetPin = 0;
    uint8_t SettingPin = 0;
    bool LogManagerEnabled = true;
    SDeviceConfig() = default;
  };

  struct SConnectionConfig {
    bool ApModeEnable = false;
    String ApSsid;
    String ApPassword;
    String ApIp;
    String ApGateway;
    String ApSubnet;
    String FallbackApSsid;
    String FallbackApPassword;
    String FallbackApIp;
    String FallbackApGateway;
    String FallbackApSubnet;
    String StaSsid;
    String StaPassword;
    bool StaAutoFallbackApEnable = true;
    uint8_t StaConnectMaxRetry = 3;
    uint32_t StaRetryDelayMs = 5 * 1000;
    bool StaIpEnable = false;
    String StaIp;
    String StaGateway;
    String StaSubnet;
    String StaPrimaryDns;
    String StaSecondaryDns;
    bool MdnsEnable = false;
    String MdnsName;
    SConnectionConfig() = default;
  };

  struct SNTPConfig {
    String Server;
    Port NtpPort {123};
    long GMTOffset = 0;
    long DaylightOffset = 0;
    String TimeZoneLabel;
    unsigned long UpdateInterval = 0;
    bool LowPowerSyncEnable = true;
    unsigned long LowPowerSyncIntervalSec = 7UL * 24UL * 60UL * 60UL;
    unsigned long LastSuccessfulSyncEpochUtc = 0;
    SNTPConfig() = default;
  };

  struct SDisplayConfig {
    int32_t Width = 0;
    int32_t Height = 0;
    Percentage JpgBrightness {30};
    Percentage JpgContrast {35};
    Percentage JpgGamma {135};
    String ImagesDir;
    String ImageExt;
    String CurrentFile;
    unsigned long ImageUpdatedAt = 0;
    SDisplayConfig() = default;
  };

  struct STimerConfig {
    ETimerWakeUp WakeUp;
    uint8_t WakeUpHour = 6;
    STimerConfig() = default;
  };

  struct STelnetConfig{
    bool Enable = false;
    Port TelnetPort {23};
    String Username;
    String Password;
    unsigned long Session = 0;
    STelnetConfig() = default;
  };

  struct SFTPConfig {
    bool Enable = false;
    Port FtpPort {21};
    String Username;
    String Password;
    SFTPConfig() = default;
  };

  struct SStorageConfig {
    EFileSystemType DefaultFileSystem = EFileSystemType::LittleFS;
    bool FallbackEnabled = true;
    SStorageConfig() = default;
  };

  struct SToneConfig {
    bool Enable = true;
    SToneConfig() = default;
  };

  struct SAppConfig {
    SDeviceConfig Device {};
    SNTPConfig Ntp {};
    SConnectionConfig Connection {};
    SDisplayConfig Display {};
    STimerConfig Timer {};
    STelnetConfig Telnet {};
    SFTPConfig Ftp {};
    SStorageConfig Storage {};
    SToneConfig Tone {};
    SAppConfig() = default;
  };

  constexpr unsigned long BAUDRATE = 115200;

  constexpr const char *LOGS_DIR = "logs";

  constexpr const char *IMAGES_DIR = "images";
  constexpr const char *CONFIG_FILE = "config.ini";

  constexpr const char *FIRMWARE_DIR = "firmware";
  constexpr const char *FIRMWARE_PATH = "firmware/firmware.bin";
  constexpr const char *FIRMWARE_SHA_PATH = "firmware/firmware.sha256";

  constexpr EFileSystemType DEFAULT_FILE_SYSTEM = EFileSystemType::SDCard;
  constexpr bool STORAGE_FALLBACK_ENABLED = true;

  constexpr uint16_t DISPLAY_WIDTH = 960;
  constexpr uint16_t DISPLAY_HEIGHT = 540;
  constexpr const char *IMAGE_EXT = ".jpg";

  constexpr uint8_t BATTERY_PIN = static_cast<uint8_t>(EDevicePins::BatPin);
  constexpr uint8_t SETTING_PIN = static_cast<uint8_t>(EDevicePins::Btn1);
  constexpr uint8_t RESET_PIN = static_cast<uint8_t>(EDevicePins::Btn2);
  constexpr uint8_t TONE_PIN = static_cast<uint8_t>(EDevicePins::PiezoPin);

  static constexpr uint8_t RTC_ADDRESS = 0x51;
  constexpr uint8_t RTC_SDA_PIN = static_cast<uint8_t>(EDevicePins::RTCSdaPin);
  constexpr uint8_t RTC_SCL_PIN = static_cast<uint8_t>(EDevicePins::RTCSclPin);

  constexpr uint8_t SD_MISO_PIN = static_cast<uint8_t>(EDevicePins::SDMisoPin);
  constexpr uint8_t SD_MOSI_PIN = static_cast<uint8_t>(EDevicePins::SDMosiPin);
  constexpr uint8_t SD_SCK_PIN = static_cast<uint8_t>(EDevicePins::SDSckPin);
  constexpr uint8_t SD_CS_PIN = static_cast<uint8_t>(EDevicePins::SDCsPin);
  
  constexpr uint32_t SECONDS_PER_MINUTE = 60;
  constexpr uint32_t SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE;
  constexpr uint32_t SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR;

  constexpr uint32_t KB = 1024;
  constexpr size_t LOOP_TASK_STACK_SIZE = 48 * KB;
  constexpr size_t JPEG_DECODE_TASK_STACK_SIZE = 32 * KB;  
  constexpr size_t BUTTON_TASK_STACK_SIZE = 16 * KB;
  constexpr size_t TELNET_TASK_STACK_SIZE = 8 * KB;
  constexpr size_t FTP_TASK_STACK_SIZE = 8 * KB;
 
  constexpr size_t ONE_SECOND_MS = 1000;
  constexpr uint32_t REBOOT_LONG_PRESS_MS = 3 * ONE_SECOND_MS;
  constexpr uint32_t FACTORY_RESET_LONG_PRESS_MS = 30 * ONE_SECOND_MS;

  constexpr uint32_t DELAY_ULTRA_SHORT_MS = ONE_SECOND_MS / 100;
  constexpr uint32_t DELAY_SHORT_MS = ONE_SECOND_MS / 10;
  constexpr uint32_t DELAY_MEDIUM_MS = ONE_SECOND_MS / 5;
  constexpr uint32_t DELAY_HALF_SEC_MS = ONE_SECOND_MS / 2;
  constexpr uint32_t DELAY_ONE_SEC_MS = ONE_SECOND_MS;

  constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 30 * ONE_SECOND_MS;
  constexpr uint32_t MAINTENANCE_INACTIVITY_TIMEOUT_MS = 15 * 60 * ONE_SECOND_MS;
  constexpr uint32_t WIFI_RETRY_COUNT = 20;
  constexpr uint32_t NVS_RETRY_DELAY_MS = ONE_SECOND_MS / 10;
  constexpr uint32_t CONFIG_RETRY_DELAY_MS = ONE_SECOND_MS;

  extern RTC_DATA_ATTR uint32_t gBootCount;

}

#include <App/Utils.h>
#include <App/Configuration.h>
#include <App/Storages/LittleFS.h>
#include <App/Storages/SDCard.h>
#include <App/Storage.h>
#include <App/LogManager.h>
#include <App/NTP.h>
#include <App/RTC.h>
#include <App/Connection.h>
#include <App/Button.h>
#include <App/Tone.h>
#include <App/Tone/MaintenanceTone.h>
#include <App/Tone/LowBatteryTone.h>
#include <App/Display.h>
#include <App/FTP.h>
#include <App/Firmware.h>
#include <App/Telnet/Command.h>
#include <App/Telnet/Commands/CallbackCommand.h>
#include <App/Telnet.h>
#include <App/Fonts/OpenSans11.h>
#include <App/Fonts/OpenSans11b.h>
#include <App/Fonts/OpenSans13.h>
#include <App/Fonts/OpenSans13b.h>
#include <App/Images/DefaultImage.h>

#define CFG Configuration_::Instance()
#define UTL Utils_::Instance()
#define LFS LittleFS_::Instance()
#define SDC SDCard_::Instance()
#define STG Storage_::Instance()
#define NTP NTP_::Instance()
#define RTC RTC_::Instance()
#define CON Connection_::Instance()
#define BTN Button_::Instance()
#define TON Tone_::Instance()
#define DSP Display_::Instance()
#define FTP FTP_::Instance()
#define FWU Firmware_::Instance()
#define TLN Telnet_::Instance()
#define LOG LogManager_::Instance()

#endif