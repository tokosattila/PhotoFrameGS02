#include <App/Configuration.h>

namespace App {

  bool Configuration_::MatchIniSection(const SConfigKeyMappingEntry &tEntry, const char *tFileSection) {
    if (strcasecmp(tEntry.IniSection, tFileSection) == 0) return true;
    if ((strcasecmp(tEntry.IniSection, "ap mode fallback") == 0 && strcasecmp(tFileSection, "ap mode") == 0) ||
        (strcasecmp(tEntry.IniSection, "ap mode") == 0 && strcasecmp(tFileSection, "ap mode fallback") == 0)) {
      return true;
    }
    return false;
  }

  const std::vector<SConfigKeyMappingEntry> &Configuration_::GetKeyMapping() {
    static const SConfigKeyMappingEntry tKeyMapping[] = {
      {Configuration_::kNvsDeviceAppName, "appname", "device", EConfigType::STRING},
      {Configuration_::kNvsDeviceVersion, "version", "device", EConfigType::STRING},
      {Configuration_::kNvsDisplayBrightness, "jpg_brightness", "display", EConfigType::UCHAR},
      {Configuration_::kNvsDisplayContrast, "jpg_contrast", "display", EConfigType::UCHAR},
      {Configuration_::kNvsDisplayGamma, "jpg_gamma", "display", EConfigType::UCHAR},
      {Configuration_::kNvsDisplayFile, "image_file", "display", EConfigType::STRING},
      {Configuration_::kNvsTimeServer, "ntp_server", "ntp", EConfigType::STRING},
      {Configuration_::kNvsTimePort, "ntp_port", "ntp", EConfigType::USHORT},
      {Configuration_::kNvsTimeGmtOffset, "ntp_gmt_offset", "ntp", EConfigType::INT},
      {Configuration_::kNvsTimeUpdate, "ntp_update", "ntp", EConfigType::UINT},
      {Configuration_::kNvsConApEnable, "ap_enable", "ap mode", EConfigType::BOOL},
      {Configuration_::kNvsConApSsid, "ap_ssid", "ap mode", EConfigType::STRING},
      {Configuration_::kNvsConApPass, "ap_password", "ap mode", EConfigType::STRING},
      {Configuration_::kNvsConApIp, "ap_ip", "ap mode", EConfigType::STRING},
      {Configuration_::kNvsConApGw, "ap_gateway", "ap mode", EConfigType::STRING},
      {Configuration_::kNvsConApSubnet, "ap_subnet", "ap mode", EConfigType::STRING},
      {Configuration_::kNvsConFallbackApSsid, "fallback_ap_ssid", "ap mode fallback", EConfigType::STRING},
      {Configuration_::kNvsConFallbackApPass, "fallback_ap_password", "ap mode fallback", EConfigType::STRING},
      {Configuration_::kNvsConFallbackApIp, "fallback_ap_ip", "ap mode fallback", EConfigType::STRING},
      {Configuration_::kNvsConFallbackApGw, "fallback_ap_gateway", "ap mode fallback", EConfigType::STRING},
      {Configuration_::kNvsConFallbackApSubnet, "fallback_ap_subnet", "ap mode fallback", EConfigType::STRING},
      {Configuration_::kNvsConStaSsid, "sta_ssid", "sta mode", EConfigType::STRING},
      {Configuration_::kNvsConStaPass, "sta_password", "sta mode", EConfigType::STRING},
      {Configuration_::kNvsConStaAutoFallback, "sta_auto_fallback", "sta mode", EConfigType::BOOL},
      {Configuration_::kNvsConStaMaxRetry, "sta_max_retry", "sta mode", EConfigType::UCHAR},
      {Configuration_::kNvsConStaRetryDelayMs, "sta_retry_delay_ms", "sta mode", EConfigType::UINT},
      {Configuration_::kNvsConStaEnable, "sta_enable", "static ip", EConfigType::BOOL},
      {Configuration_::kNvsConStaIp, "sta_ip", "static ip", EConfigType::STRING},
      {Configuration_::kNvsConStaGw, "sta_gateway", "static ip", EConfigType::STRING},
      {Configuration_::kNvsConStaSubnet, "sta_subnet", "static ip", EConfigType::STRING},
      {Configuration_::kNvsConStaDns1, "sta_dns1", "static ip", EConfigType::STRING},
      {Configuration_::kNvsConStaDns2, "sta_dns2", "static ip", EConfigType::STRING},
      {Configuration_::kNvsConMdnsEnable, "mdns_enable", "mdns", EConfigType::BOOL},
      {Configuration_::kNvsConMdnsName, "mdns_hostname", "mdns", EConfigType::STRING},
      {Configuration_::kNvsTimeGmtOffsetLong, "ntp_gmt_offset", "ntp", EConfigType::INT},
      {Configuration_::kNvsTimeDaylightOffset, "ntp_daylight_offset", "ntp", EConfigType::INT},
      {Configuration_::kNvsTimeZoneLabel, "ntp_timezone_label", "ntp", EConfigType::STRING},
      {Configuration_::kNvsTimeLowPowerSyncEnable, "ntp_low_power_sync_enable", "ntp", EConfigType::BOOL},
      {Configuration_::kNvsTimeLowPowerSyncInterval, "ntp_low_power_sync_interval", "ntp", EConfigType::ULONG},
      {Configuration_::kNvsTimeLastSuccessfulSync, "ntp_last_successful_sync", "ntp", EConfigType::ULONG},
      {Configuration_::kNvsTimerWake, "wake_up", "timer", EConfigType::UCHAR},
      {Configuration_::kNvsTimerWakeHour, "wake_up_hour", "timer", EConfigType::UCHAR},
      {Configuration_::kNvsTelnetEnable, "telnet_enable", "telnet", EConfigType::BOOL},
      {Configuration_::kNvsTelnetPort, "telnet_port", "telnet", EConfigType::UCHAR},
      {Configuration_::kNvsTelnetUsername, "telnet_username", "telnet", EConfigType::STRING},
      {Configuration_::kNvsTelnetPassword, "telnet_password", "telnet", EConfigType::STRING},
      {Configuration_::kNvsTelnetSession, "telnet_session", "telnet", EConfigType::ULONG},
      {Configuration_::kNvsFtpEnable, "ftp_enable", "ftp", EConfigType::BOOL},
      {Configuration_::kNvsFtpPort, "ftp_port", "ftp", EConfigType::UCHAR},
      {Configuration_::kNvsFtpUsername, "ftp_username", "ftp", EConfigType::STRING},
      {Configuration_::kNvsFtpPassword, "ftp_password", "ftp", EConfigType::STRING},
      {Configuration_::kNvsDeviceLogEnable, "log_enabled", "device", EConfigType::BOOL},
      {"", "default_file_system", "storage", EConfigType::GLOBAL_INT},
      {"", "fallback_enabled", "storage", EConfigType::GLOBAL_INT},
      {"", "config_file", "device", EConfigType::GLOBAL_INT},
      {"", "battery_pin", "device", EConfigType::GLOBAL_INT},
      {"", "setting_pin", "device", EConfigType::GLOBAL_INT},
      {"", "reset_pin", "device", EConfigType::GLOBAL_INT},
      {"", "display_width", "display", EConfigType::GLOBAL_INT},
      {"", "display_height", "display", EConfigType::GLOBAL_INT},
      {"", "image_ext", "display", EConfigType::GLOBAL_INT},
      {"", "images_dir", "display", EConfigType::GLOBAL_INT},
      {"", "wake_pin", "timer", EConfigType::GLOBAL_INT},
    };
    static const std::vector<SConfigKeyMappingEntry> tKeyMappingVector(tKeyMapping, tKeyMapping + (sizeof(tKeyMapping) / sizeof(tKeyMapping[0])));
    return tKeyMappingVector;
  }

  bool Configuration_::ParseLine(char *tLine, char *tSection, char *tKey, char *tValue) {
    if (tLine == nullptr || tLine[0] == '\0') return false;
    TrimValue(tLine);
    if (tLine[0] == '\0' || tLine[0] == '#' || tLine[0] == ';') return false;
    if (tLine[0] == '[') {
      char *tEnd = strchr(tLine, ']');
      if (tEnd) {
        *tEnd = '\0';
        strncpy(tSection, tLine + 1, 31);
        tSection[31] = '\0';
      }
      return false;
    }
    char *tSeparator = strchr(tLine, '=');
    if (tSeparator) {
      *tSeparator = '\0';
      strncpy(tKey, tLine, 31);
      tKey[31] = '\0';
      strncpy(tValue, tSeparator + 1, 127);
      tValue[127] = '\0';
      TrimValue(tKey);
      TrimValue(tValue);
      return true;
    }
    return false;
  }

  void Configuration_::TrimValue(char *tValue) {
    if (!tValue || tValue[0] == '\0') return;
    size_t tLength = strlen(tValue);
    while (tLength > 0 && (tValue[tLength - 1] == ' ' || tValue[tLength - 1] == '\t' || tValue[tLength - 1] == '\r' || tValue[tLength - 1] == '\n')) tValue[--tLength] = '\0';
    size_t tStart = 0;
    while (tValue[tStart] == ' ' || tValue[tStart] == '\t' || tValue[tStart] == '\r' || tValue[tStart] == '\n') tStart++;
    if (tStart > 0) memmove(tValue, tValue + tStart, tLength - tStart + 1);
  }

  void Configuration_::ApplyINIValue(SAppConfig &tConfig, const char *tSection, const SConfigKeyMappingEntry &tEntry, const char *tValue) {
    if (strcasecmp(tSection, "device") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsDeviceAppName) == 0) tConfig.Device.Name = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsDeviceVersion) == 0) tConfig.Device.Version = String(tValue);
    } else if (strcasecmp(tSection, "display") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsDisplayBrightness) == 0) tConfig.Display.JpgBrightness = Percentage(static_cast<uint8_t>(atoi(tValue)));
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsDisplayContrast) == 0) tConfig.Display.JpgContrast = Percentage(static_cast<uint8_t>(atoi(tValue)));
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsDisplayGamma) == 0) tConfig.Display.JpgGamma = Percentage(static_cast<uint8_t>(atoi(tValue)));
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsDisplayFile) == 0) tConfig.Display.CurrentFile = String(tValue);
    } else if (strcasecmp(tSection, "ntp") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimeServer) == 0) tConfig.Ntp.Server = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimePort) == 0) tConfig.Ntp.NtpPort = Port(static_cast<uint16_t>(atoi(tValue)));
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimeGmtOffset) == 0 || strcmp(tEntry.NvsKey, Configuration_::kNvsTimeGmtOffsetLong) == 0) tConfig.Ntp.GMTOffset = atol(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimeDaylightOffset) == 0) tConfig.Ntp.DaylightOffset = atol(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimeZoneLabel) == 0) tConfig.Ntp.TimeZoneLabel = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimeUpdate) == 0) tConfig.Ntp.UpdateInterval = UTL.SafeAtoul(tValue, 60, 86400000, 3600);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimeLowPowerSyncEnable) == 0) tConfig.Ntp.LowPowerSyncEnable = (strcasecmp(tValue, "true") == 0 || atoi(tValue) != 0);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimeLowPowerSyncInterval) == 0) tConfig.Ntp.LowPowerSyncIntervalSec = UTL.SafeAtoul(tValue, 60, 31536000, 604800);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimeLastSuccessfulSync) == 0) tConfig.Ntp.LastSuccessfulSyncEpochUtc = strtoul(tValue, nullptr, 10);
    } else if (strcasecmp(tSection, "ap mode") == 0 || strcasecmp(tSection, "ap mode fallback") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsConApEnable) == 0) tConfig.Connection.ApModeEnable = (strcasecmp(tValue, "true") == 0 || atoi(tValue) != 0);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConApSsid) == 0) tConfig.Connection.ApSsid = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConApPass) == 0) tConfig.Connection.ApPassword = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConApIp) == 0) tConfig.Connection.ApIp = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConApGw) == 0) tConfig.Connection.ApGateway = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConApSubnet) == 0) tConfig.Connection.ApSubnet = String(tValue);
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsConFallbackApSsid) == 0) tConfig.Connection.FallbackApSsid = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConFallbackApPass) == 0) tConfig.Connection.FallbackApPassword = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConFallbackApIp) == 0) tConfig.Connection.FallbackApIp = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConFallbackApGw) == 0) tConfig.Connection.FallbackApGateway = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConFallbackApSubnet) == 0) tConfig.Connection.FallbackApSubnet = String(tValue);
    } else if (strcasecmp(tSection, "sta mode") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaSsid) == 0) tConfig.Connection.StaSsid = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaPass) == 0) tConfig.Connection.StaPassword = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaAutoFallback) == 0) tConfig.Connection.StaAutoFallbackApEnable = (strcasecmp(tValue, "true") == 0 || atoi(tValue) != 0);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaMaxRetry) == 0) tConfig.Connection.StaConnectMaxRetry = static_cast<uint8_t>(atoi(tValue));
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaRetryDelayMs) == 0) tConfig.Connection.StaRetryDelayMs = static_cast<uint32_t>(strtoul(tValue, nullptr, 10));
    } else if (strcasecmp(tSection, "static ip") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaEnable) == 0) tConfig.Connection.StaIpEnable = (strcasecmp(tValue, "true") == 0 || atoi(tValue) != 0);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaIp) == 0) tConfig.Connection.StaIp = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaGw) == 0) tConfig.Connection.StaGateway = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaSubnet) == 0) tConfig.Connection.StaSubnet = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaDns1) == 0) tConfig.Connection.StaPrimaryDns = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConStaDns2) == 0) tConfig.Connection.StaSecondaryDns = String(tValue);
    } else if (strcasecmp(tSection, "mdns") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsConMdnsEnable) == 0) tConfig.Connection.MdnsEnable = (strcasecmp(tValue, "true") == 0 || atoi(tValue) != 0);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsConMdnsName) == 0) tConfig.Connection.MdnsName = String(tValue);
    } else if (strcasecmp(tSection, "timer") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimerWake) == 0) tConfig.Timer.WakeUp = static_cast<ETimerWakeUp>(atoi(tValue));
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTimerWakeHour) == 0) tConfig.Timer.WakeUpHour = static_cast<uint8_t>(atoi(tValue)) % 24;
    } else if (strcasecmp(tSection, "telnet") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsTelnetEnable) == 0) tConfig.Telnet.Enable = (strcasecmp(tValue, "true") == 0 || atoi(tValue) != 0);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTelnetPort) == 0) tConfig.Telnet.TelnetPort = Port(static_cast<uint8_t>(atoi(tValue)));
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTelnetUsername) == 0) tConfig.Telnet.Username = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTelnetPassword) == 0) tConfig.Telnet.Password = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsTelnetSession) == 0) tConfig.Telnet.Session = UTL.SafeAtoul(tValue, 60, 86400000, 3600000);
    } else if (strcasecmp(tSection, "ftp") == 0) {
      if (strcmp(tEntry.NvsKey, Configuration_::kNvsFtpEnable) == 0) tConfig.Ftp.Enable = (strcasecmp(tValue, "true") == 0 || atoi(tValue) != 0);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsFtpPort) == 0) tConfig.Ftp.FtpPort = Port(static_cast<uint8_t>(atoi(tValue)));
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsFtpUsername) == 0) tConfig.Ftp.Username = String(tValue);
      else if (strcmp(tEntry.NvsKey, Configuration_::kNvsFtpPassword) == 0) tConfig.Ftp.Password = String(tValue);
    }
  }

  bool Configuration_::ReadINIFile(const char *tFileName, SAppConfig &tConfig) {
    File tFile = STG.OpenFile(tFileName, FILE_READ);
    if (!tFile) {
      xLOG("INI file (%s) open failed!", tFileName);
      return false;
    }
    char tLine[256];
    char tSection[32] = {};
    char tKey[32];
    char tValue[128];
    const auto &tMapping = GetKeyMapping();
    while (tFile.available()) {
      String tLineStr = tFile.readStringUntil('\n');
      if (tLineStr.length() == 0) continue;
      if (tLineStr.endsWith("\r")) tLineStr.remove(tLineStr.length() - 1);
      strncpy(tLine, tLineStr.c_str(), sizeof(tLine) - 1);
      tLine[sizeof(tLine) - 1] = '\0';
      if (!ParseLine(tLine, tSection, tKey, tValue)) continue;
      auto tIt = std::find_if(tMapping.begin(), tMapping.end(), [tSection, tKey](const SConfigKeyMappingEntry &tEntry) {
        return MatchIniSection(tEntry, tSection) && strcasecmp(tEntry.IniKey, tKey) == 0;
      });
      if (tIt != tMapping.end()) ApplyINIValue(tConfig, tSection, *tIt, tValue);
    }
    tFile.close();
    return true;
  }

  Configuration_ &Configuration_::Instance() {
    static Configuration_ tInstance;
    return tInstance;
  }

  Configuration_::Configuration_() {
    mMutex = xSemaphoreCreateRecursiveMutex();
  }

  Configuration_::~Configuration_() {
    if (mMutex) {
      vSemaphoreDelete(mMutex);
      mMutex = nullptr;
    }
  }

  void Configuration_::Lock() {
    if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
  }

  void Configuration_::Unlock() {
    if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
  }

  bool Configuration_::Begin(bool tReadOnly) {
    const uint8_t kMaxRetry = 3;
    for (uint8_t tIndex = 0; tIndex < kMaxRetry; tIndex++) {
      if (mConfig.begin(mLabel, tReadOnly, mPartLabel)) return true;
      vTaskDelay(NVS_RETRY_DELAY_MS / portTICK_PERIOD_MS);
    }
    return false;
  }

  void Configuration_::End() {
    mConfig.end();
  }

  void Configuration_::AccessConfig(bool tReadOnly, std::function<void()> tAction) {
    Guard tLock;
    if (!Begin(tReadOnly)) {
      xLOG("Config failed %s -> %s", (tReadOnly ? "read" : "write"), mLabel);
      return;
    }
    tAction();
    End();
  }

  bool Configuration_::Init() {
  #if !PRODUCTION
    if (!xLOG_S) xLOG_B(BAUDRATE);
    while (!xLOG_S) vTaskDelay(DELAY_ULTRA_SHORT_MS / portTICK_PERIOD_MS);
    xLOG_PL();
  #endif  
    Guard tLock;
    if (!Begin(false)) {
      xLOG("Config begin failed. Retrying in 1s...");
      vTaskDelay(CONFIG_RETRY_DELAY_MS / portTICK_PERIOD_MS);
      if (!Begin(false)) {
        xLOG("Config begin failed after retry. Cannot proceed.");
        return false;
      }
    }
    bool tConfigExists = mConfig.getBool(kNvsDeviceConfig);
    End();
    if (!tConfigExists) {
      xLOG("Attempting to create default config...");
      if (!CreateConfig()) {
        xLOG("Failed to create default config.");
        return false;
      }
    }
    return true;
  }

  SAppConfig Configuration_::GetDefaultConfig() {
    SAppConfig tDefaultConfig {};
    tDefaultConfig.Device.Name = "PHOTO FRAME GS02";
    tDefaultConfig.Device.Version = "v1.0";
    tDefaultConfig.Device.ConfigFile = CONFIG_FILE;
    tDefaultConfig.Device.BatteryPin = BATTERY_PIN;
    tDefaultConfig.Device.ResetPin = RESET_PIN;
    tDefaultConfig.Device.SettingPin = SETTING_PIN;
    tDefaultConfig.Device.LogManagerEnabled = true;
    tDefaultConfig.Display.Width = DISPLAY_WIDTH;
    tDefaultConfig.Display.Height = DISPLAY_HEIGHT;
    tDefaultConfig.Display.JpgBrightness = Percentage(25);
    tDefaultConfig.Display.JpgContrast = Percentage(75);
    tDefaultConfig.Display.JpgGamma = Percentage(125);
    tDefaultConfig.Display.ImagesDir = IMAGES_DIR;
    tDefaultConfig.Display.ImageExt = IMAGE_EXT;
    tDefaultConfig.Display.CurrentFile = "";
    tDefaultConfig.Display.ImageUpdatedAt = 0;
    tDefaultConfig.Ntp.Server = "ro.pool.ntp.org";
    tDefaultConfig.Ntp.NtpPort = Port(123);
    tDefaultConfig.Ntp.GMTOffset = 2 * 60 * 60;
    tDefaultConfig.Ntp.DaylightOffset = 1 * 60 * 60;
    tDefaultConfig.Ntp.TimeZoneLabel = "EET";
    tDefaultConfig.Ntp.UpdateInterval = 60 * 1000;
    tDefaultConfig.Ntp.LowPowerSyncEnable = true;
    tDefaultConfig.Ntp.LowPowerSyncIntervalSec = 7UL * 24UL * 60UL * 60UL;
    tDefaultConfig.Ntp.LastSuccessfulSyncEpochUtc = 0;
    tDefaultConfig.Connection.ApModeEnable = true;
    tDefaultConfig.Connection.ApSsid = "PhotoFrameGS02";
    tDefaultConfig.Connection.ApPassword = "123456789";
    tDefaultConfig.Connection.ApIp = "192.168.4.1";
    tDefaultConfig.Connection.ApGateway = "192.168.4.1";
    tDefaultConfig.Connection.ApSubnet = "255.255.255.0";
    tDefaultConfig.Connection.FallbackApSsid = "PhotoFrameGS02-Fallback";
    tDefaultConfig.Connection.FallbackApPassword = "123456789";
    tDefaultConfig.Connection.FallbackApIp = "192.168.5.1";
    tDefaultConfig.Connection.FallbackApGateway = "192.168.5.1";
    tDefaultConfig.Connection.FallbackApSubnet = "255.255.255.0";
    tDefaultConfig.Connection.StaSsid = "SSID";
    tDefaultConfig.Connection.StaPassword = "PASSWORD";
    tDefaultConfig.Connection.StaAutoFallbackApEnable = true;
    tDefaultConfig.Connection.StaConnectMaxRetry = 3;
    tDefaultConfig.Connection.StaRetryDelayMs = 5000;
    tDefaultConfig.Connection.StaIpEnable = false;
    tDefaultConfig.Connection.StaIp = "192.168.0.83";
    tDefaultConfig.Connection.StaGateway = "192.168.0.1";
    tDefaultConfig.Connection.StaSubnet = "255.255.255.0";
    tDefaultConfig.Connection.StaPrimaryDns = "192.168.0.1";
    tDefaultConfig.Connection.StaSecondaryDns = "8.8.8.8";
    tDefaultConfig.Connection.MdnsEnable = false;
    tDefaultConfig.Connection.MdnsName = "photoframegs02";
    tDefaultConfig.Timer.WakeUp = ETimerWakeUp::Daily;
    tDefaultConfig.Timer.WakeUpHour = 6;
    tDefaultConfig.Telnet.Enable = true;
    tDefaultConfig.Telnet.TelnetPort = Port(23);
    tDefaultConfig.Telnet.Username = "admin";
    tDefaultConfig.Telnet.Password = "123456789";
    tDefaultConfig.Telnet.Session = 3600000;
    tDefaultConfig.Ftp.Enable = true;
    tDefaultConfig.Ftp.FtpPort = Port(21);
    tDefaultConfig.Ftp.Username = "admin";
    tDefaultConfig.Ftp.Password = "123456789";
    return tDefaultConfig;
  }

  bool Configuration_::CreateConfig() {
    SAppConfig tDefaultConfig = GetDefaultConfig();
    if (!SaveAllConfig(tDefaultConfig)) {
      xLOG("Failed to save default config values!");
      return false;
    }
    bool tSuccess = false;
    AccessConfig(false, [&]() {
      tSuccess = mConfig.putBool(kNvsDeviceConfig, true);
    });
    if (!tSuccess) {
      xLOG("Config creation failed, NVS write access failure!");
      return false;
    }
    xLOG("Default config created successfully!");
    return true;
  }

  bool Configuration_::FactoryReset() {
    xLOG("Starting factory reset...");
    Guard tLock;
    if (!Begin(false)) {
      xLOG("Failed to open NVS for factory reset!");
      return false;
    }
    if (!mConfig.clear()) {
      xLOG("Failed to clear NVS storage!");
      End();
      return false;
    }
    xLOG("NVS storage cleared successfully!");
    End();
    if (!CreateConfig()) {
      xLOG("Failed to recreate default configuration!");
      return false;
    }
    AccessConfig(false, [&]() {
      mConfig.putBool(kNvsDeviceConfig, true);
    });
    xLOG("Factory reset completed successfully!");
    return true;
  }

  template<> SDeviceConfig Configuration_::Get<SDeviceConfig>() {
    SDeviceConfig tCfg {};
    AccessConfig(true, [&]() {
      tCfg.Name = mConfig.getString(kNvsDeviceAppName, "PHOTO FRAME GS02");
      tCfg.Version = mConfig.getString(kNvsDeviceVersion, "v1.0");
      tCfg.ConfigFile = CONFIG_FILE;
      tCfg.BatteryPin = BATTERY_PIN;
      tCfg.ResetPin = RESET_PIN;
      tCfg.SettingPin = SETTING_PIN;
      tCfg.LogManagerEnabled = mConfig.getBool(kNvsDeviceLogEnable, true);
    });
    return tCfg;
  }

  template<> SNTPConfig Configuration_::Get<SNTPConfig>() {
    SNTPConfig tCfg {};
    AccessConfig(true, [&]() {
      tCfg.Server = mConfig.getString(kNvsTimeServer, "ro.pool.ntp.org");
      tCfg.NtpPort = Port(mConfig.getUShort(kNvsTimePort, 123));
      tCfg.GMTOffset = mConfig.getInt(kNvsTimeGmtOffset, 2 * 60 * 60);
      tCfg.DaylightOffset = mConfig.getInt(kNvsTimeDaylightOffset, 1 * 60 * 60);
      tCfg.TimeZoneLabel = mConfig.getString(kNvsTimeZoneLabel, "EET");
      tCfg.UpdateInterval = mConfig.getUInt(kNvsTimeUpdate, 60 * 1000);
      tCfg.LowPowerSyncEnable = mConfig.getBool(kNvsTimeLowPowerSyncEnable, true);
      tCfg.LowPowerSyncIntervalSec = mConfig.getULong(kNvsTimeLowPowerSyncInterval, 7UL * 24UL * 60UL * 60UL);
      tCfg.LastSuccessfulSyncEpochUtc = mConfig.getULong(kNvsTimeLastSuccessfulSync, 0);
    });
    return tCfg;
  }

  template<> SConnectionConfig Configuration_::Get<SConnectionConfig>() {
    SConnectionConfig tCfg {};
    AccessConfig(true, [&]() {
      tCfg.ApModeEnable = mConfig.getBool(kNvsConApEnable, false);
      tCfg.ApSsid = mConfig.getString(kNvsConApSsid, "PhotoFrame GS02");
      tCfg.ApPassword = mConfig.getString(kNvsConApPass, "123456789");
      tCfg.ApIp = mConfig.getString(kNvsConApIp, "192.168.4.1");
      tCfg.ApGateway = mConfig.getString(kNvsConApGw, "192.168.4.1");
      tCfg.ApSubnet = mConfig.getString(kNvsConApSubnet, "255.255.255.0");
      tCfg.FallbackApSsid = mConfig.getString(kNvsConFallbackApSsid, "PhotoFrameGS02-Fallback");
      tCfg.FallbackApPassword = mConfig.getString(kNvsConFallbackApPass, "123456789");
      tCfg.FallbackApIp = mConfig.getString(kNvsConFallbackApIp, "192.168.5.1");
      tCfg.FallbackApGateway = mConfig.getString(kNvsConFallbackApGw, "192.168.5.1");
      tCfg.FallbackApSubnet = mConfig.getString(kNvsConFallbackApSubnet, "255.255.255.0");
      tCfg.StaSsid = mConfig.getString(kNvsConStaSsid, "");
      tCfg.StaPassword = mConfig.getString(kNvsConStaPass, "");
      tCfg.StaAutoFallbackApEnable = mConfig.getBool(kNvsConStaAutoFallback, true);
      tCfg.StaConnectMaxRetry = mConfig.getUChar(kNvsConStaMaxRetry, 3);
      tCfg.StaRetryDelayMs = mConfig.getUInt(kNvsConStaRetryDelayMs, 5000);
      tCfg.StaIpEnable = mConfig.getBool(kNvsConStaEnable, false);
      tCfg.StaIp = mConfig.getString(kNvsConStaIp, "");
      tCfg.StaGateway = mConfig.getString(kNvsConStaGw, "");
      tCfg.StaSubnet = mConfig.getString(kNvsConStaSubnet, "");
      tCfg.StaPrimaryDns = mConfig.getString(kNvsConStaDns1, "");
      tCfg.StaSecondaryDns = mConfig.getString(kNvsConStaDns2, "");
      tCfg.MdnsEnable = mConfig.getBool(kNvsConMdnsEnable, false);
      tCfg.MdnsName = mConfig.getString(kNvsConMdnsName, "photoframegs02");
    });
    return tCfg;
  }

  template<> SDisplayConfig Configuration_::Get<SDisplayConfig>() {
    SDisplayConfig tCfg {};
    AccessConfig(true, [&]() {
      tCfg.Width = DISPLAY_WIDTH;
      tCfg.Height = DISPLAY_HEIGHT;
      tCfg.JpgBrightness = Percentage(mConfig.getUChar(kNvsDisplayBrightness, 30));
      tCfg.JpgContrast = Percentage(mConfig.getUChar(kNvsDisplayContrast, 35));
      tCfg.JpgGamma = Percentage(mConfig.getUChar(kNvsDisplayGamma, 135));
      tCfg.ImagesDir = IMAGES_DIR;
      tCfg.ImageExt = IMAGE_EXT;
      tCfg.CurrentFile = mConfig.getString(kNvsDisplayFile, "");
      tCfg.ImageUpdatedAt = mConfig.getULong(kNvsDisplayImageUpdatedAt, 0);
    });
    return tCfg;
  }

  template<> STimerConfig Configuration_::Get<STimerConfig>() {
    STimerConfig tCfg {};
    AccessConfig(true, [&]() {
      tCfg.WakeUp = static_cast<ETimerWakeUp>(mConfig.getUChar(kNvsTimerWake, static_cast<uint8_t>(ETimerWakeUp::Daily)));
      tCfg.WakeUpHour = mConfig.getUChar(kNvsTimerWakeHour, 6);
    });
    return tCfg;
  }

  template<> STelnetConfig Configuration_::Get<STelnetConfig>() {
    STelnetConfig tCfg {};
    AccessConfig(true, [&]() {
      tCfg.Enable = mConfig.getBool(kNvsTelnetEnable, true);
      tCfg.TelnetPort = Port(mConfig.getUChar(kNvsTelnetPort, 23));
      tCfg.Username = mConfig.getString(kNvsTelnetUsername, "admin");
      tCfg.Password = mConfig.getString(kNvsTelnetPassword, "123456789");
      tCfg.Session = mConfig.getULong(kNvsTelnetSession, 3600000);
    });
    return tCfg;
  }

  template<> SFTPConfig Configuration_::Get<SFTPConfig>() {
    SFTPConfig tCfg {};
    AccessConfig(true, [&]() {
      tCfg.Enable = mConfig.getBool(kNvsFtpEnable, true);
      tCfg.FtpPort = Port(mConfig.getUChar(kNvsFtpPort, 21));
      tCfg.Username = mConfig.getString(kNvsFtpUsername, "admin");
      tCfg.Password = mConfig.getString(kNvsFtpPassword, "123456789");
    });
    return tCfg;
  }

  template<> SStorageConfig Configuration_::Get<SStorageConfig>() {
    SStorageConfig tCfg {};
    tCfg.DefaultFileSystem = DEFAULT_FILE_SYSTEM;
    tCfg.FallbackEnabled = STORAGE_FALLBACK_ENABLED;
    return tCfg;
  }

  template<> SAppConfig Configuration_::Get<SAppConfig>() {
    SAppConfig tCfg {};
    tCfg.Device = Get<SDeviceConfig>();
    tCfg.Ntp = Get<SNTPConfig>();
    tCfg.Connection = Get<SConnectionConfig>();
    tCfg.Display = Get<SDisplayConfig>();
    tCfg.Timer = Get<STimerConfig>();
    tCfg.Telnet = Get<STelnetConfig>();
    tCfg.Ftp = Get<SFTPConfig>();
    tCfg.Storage = Get<SStorageConfig>();
    return tCfg;
  }

  bool Configuration_::SaveImageName(const char *tValue) {
    if (!tValue) return false;
    bool tSuccess = false;
    uint32_t tEpoch = static_cast<uint32_t>(time(nullptr));
    if (tEpoch == 0) tEpoch = RTC.GetEpoch();
    AccessConfig(false, [&]() {
      size_t tBytesWritten = mConfig.putString(kNvsDisplayFile, tValue);
      String tSaved = mConfig.getString(kNvsDisplayFile, "");
      tSuccess = (tBytesWritten > 0) || (tSaved == String(tValue));
      if (tSuccess) tSuccess = mConfig.putULong(kNvsDisplayImageUpdatedAt, tEpoch);
    });
    if (!tSuccess) xLOG("Failed to save image name → %s", (tValue && tValue[0]) ? tValue : "<empty>");
    return tSuccess;
  }

  uint32_t Configuration_::GetImageUpdatedAt() {
    uint32_t tValue = 0;
    AccessConfig(true, [&]() {
      tValue = mConfig.getULong(kNvsDisplayImageUpdatedAt, 0);
    });
    return tValue;
  }

  bool Configuration_::SaveSession(uint32_t tValue) {
    bool tSuccess = false;
    AccessConfig(false, [&]() {
      tSuccess = mConfig.putULong(kNvsTelnetAuthSession, tValue);
    });
    if (!tSuccess) xLOG("Failed to save session → %lu", tValue);
    return tSuccess;
  }

  uint32_t Configuration_::GetSession() {
    uint32_t tValue = 0;
    AccessConfig(true, [&]() {
      tValue = mConfig.getULong(kNvsTelnetAuthSession, 0);
    });
    return tValue;
  }

  void Configuration_::UpdateNTPLastSync(unsigned long tEpochUtc) {
    AccessConfig(false, [&]() {
      mConfig.putULong(kNvsTimeLastSuccessfulSync, tEpochUtc);
    });
  }

  const char *Configuration_::PrepareAllConfigToINI() {
    String tCfgIni = "";
    AccessConfig(true, [&]() {
      auto AppendSection = [&](const char *tSection) {
        if (!tCfgIni.isEmpty()) tCfgIni += "\n";
        tCfgIni += "[";
        tCfgIni += tSection;
        tCfgIni += "]\n";
      };
      auto AppendLine = [&](const char *tKey, const String &tValue) {
        tCfgIni += tKey;
        tCfgIni += " = ";
        tCfgIni += tValue;
        tCfgIni += "\n";
      };
      AppendSection("device");
      AppendLine("appname", mConfig.getString(kNvsDeviceAppName, "PHOTO FRAME GS02"));
      AppendLine("version", mConfig.getString(kNvsDeviceVersion, "v1.0"));
      AppendLine("log_enabled", mConfig.getBool(kNvsDeviceLogEnable, true) ? "true" : "false");
      AppendSection("display");
      AppendLine("jpg_brightness", String(mConfig.getUChar(kNvsDisplayBrightness, 30)));
      AppendLine("jpg_contrast", String(mConfig.getUChar(kNvsDisplayContrast, 35)));
      AppendLine("jpg_gamma", String(mConfig.getUChar(kNvsDisplayGamma, 135)));
      AppendLine("image_file", mConfig.getString(kNvsDisplayFile, ""));
      AppendSection("ntp");
      AppendLine("ntp_server", mConfig.getString(kNvsTimeServer, "ro.pool.ntp.org"));
      AppendLine("ntp_port", String(mConfig.getUShort(kNvsTimePort, 123)));
      AppendLine("ntp_gmt_offset", String(mConfig.getInt(kNvsTimeGmtOffset, 2 * 60 * 60)));
      AppendLine("ntp_daylight_offset", String(mConfig.getInt(kNvsTimeDaylightOffset, 1 * 60 * 60)));
      AppendLine("ntp_timezone_label", mConfig.getString(kNvsTimeZoneLabel, "EET"));
      AppendLine("ntp_low_power_sync_enable", mConfig.getBool(kNvsTimeLowPowerSyncEnable, true) ? "true" : "false");
      AppendLine("ntp_low_power_sync_interval", String(mConfig.getULong(kNvsTimeLowPowerSyncInterval, 7UL * 24UL * 60UL * 60UL)));
      AppendSection("ap mode");
      AppendLine("ap_enable", mConfig.getBool(kNvsConApEnable, false) ? "true" : "false");
      AppendLine("ap_ssid", mConfig.getString(kNvsConApSsid, "PhotoFrame GS02"));
      AppendLine("ap_password", mConfig.getString(kNvsConApPass, "123456789"));
      AppendLine("ap_ip", mConfig.getString(kNvsConApIp, "192.168.4.1"));
      AppendLine("ap_gateway", mConfig.getString(kNvsConApGw, "192.168.4.1"));
      AppendLine("ap_subnet", mConfig.getString(kNvsConApSubnet, "255.255.255.0"));
      AppendLine("fallback_ap_ssid", mConfig.getString(kNvsConFallbackApSsid, "PhotoFrameGS02-Fallback"));
      AppendLine("fallback_ap_password", mConfig.getString(kNvsConFallbackApPass, "123456789"));
      AppendLine("fallback_ap_ip", mConfig.getString(kNvsConFallbackApIp, "192.168.5.1"));
      AppendLine("fallback_ap_gateway", mConfig.getString(kNvsConFallbackApGw, "192.168.5.1"));
      AppendLine("fallback_ap_subnet", mConfig.getString(kNvsConFallbackApSubnet, "255.255.255.0"));
      AppendSection("sta mode");
      AppendLine("sta_ssid", mConfig.getString(kNvsConStaSsid, ""));
      AppendLine("sta_password", mConfig.getString(kNvsConStaPass, ""));
      AppendLine("sta_auto_fallback", mConfig.getBool(kNvsConStaAutoFallback, true) ? "true" : "false");
      AppendSection("static ip");
      AppendLine("sta_enable", mConfig.getBool(kNvsConStaEnable, false) ? "true" : "false");
      AppendLine("sta_ip", mConfig.getString(kNvsConStaIp, ""));
      AppendLine("sta_gateway", mConfig.getString(kNvsConStaGw, ""));
      AppendLine("sta_subnet", mConfig.getString(kNvsConStaSubnet, ""));
      AppendLine("sta_dns1", mConfig.getString(kNvsConStaDns1, ""));
      AppendLine("sta_dns2", mConfig.getString(kNvsConStaDns2, ""));
      AppendSection("mdns");
      AppendLine("mdns_enable", mConfig.getBool(kNvsConMdnsEnable, false) ? "true" : "false");
      AppendLine("mdns_hostname", mConfig.getString(kNvsConMdnsName, "photoframegs02"));
      AppendSection("timer");
      AppendLine("wake_up", String(static_cast<uint8_t>(mConfig.getUChar(kNvsTimerWake, static_cast<uint8_t>(ETimerWakeUp::Daily)))));
      AppendLine("wake_up_hour", String(mConfig.getUChar(kNvsTimerWakeHour, 6)));
      AppendSection("telnet");
      AppendLine("telnet_enable", mConfig.getBool(kNvsTelnetEnable, true) ? "true" : "false");
      AppendLine("telnet_port", String(mConfig.getUChar(kNvsTelnetPort, 23)));
      AppendLine("telnet_username", mConfig.getString(kNvsTelnetUsername, "admin"));
      AppendLine("telnet_password", mConfig.getString(kNvsTelnetPassword, "123456789"));
      AppendLine("telnet_session", String(mConfig.getULong(kNvsTelnetSession, 3600000)));
      AppendSection("ftp");
      AppendLine("ftp_enable", mConfig.getBool(kNvsFtpEnable, true) ? "true" : "false");
      AppendLine("ftp_port", String(mConfig.getUChar(kNvsFtpPort, 21)));
      AppendLine("ftp_username", mConfig.getString(kNvsFtpUsername, "admin"));
      AppendLine("ftp_password", mConfig.getString(kNvsFtpPassword, "123456789"));
      AppendSection("storage");
      AppendLine("fallback_enabled", STORAGE_FALLBACK_ENABLED ? "true" : "false");
    });
    static String sReturn;
    sReturn = tCfgIni;
    return sReturn.c_str();
  }

  const char *Configuration_::GetConfig(const char *tKey) {
    if (!tKey || tKey[0] == '\0') return "";
    String tLowerKey = tKey;
    tLowerKey.toLowerCase();
    static char sValue[128];
    sValue[0] = '\0';
    String tempValue;
    const auto &tMapping = GetKeyMapping();
    auto tIt = std::find_if(tMapping.begin(), tMapping.end(), [&tLowerKey](const SConfigKeyMappingEntry &tEntry) {
      return tLowerKey.equals(tEntry.IniKey);
    });
    if (tIt != tMapping.end()) {
      const auto &tEntry = *tIt;
      AccessConfig(true, [&]() {
        switch (tEntry.Type) {
          case EConfigType::BOOL:
            tempValue = mConfig.getBool(tEntry.NvsKey) ? "true" : "false";
            break;
          case EConfigType::UCHAR:
            tempValue = String(mConfig.getUChar(tEntry.NvsKey));
            break;
          case EConfigType::USHORT:
            tempValue = String(mConfig.getUShort(tEntry.NvsKey));
            break;
          case EConfigType::INT:
            tempValue = String(mConfig.getInt(tEntry.NvsKey));
            break;
          case EConfigType::UINT:
          case EConfigType::ULONG:
            tempValue = String(mConfig.getULong(tEntry.NvsKey));
            break;
          case EConfigType::STRING:
            tempValue = mConfig.getString(tEntry.NvsKey);
            break;
          case EConfigType::GLOBAL_INT:
            if (strcmp(tEntry.IniKey, "config_file") == 0) tempValue = CONFIG_FILE;
            else if (strcmp(tEntry.IniKey, "battery_pin") == 0) tempValue = String(BATTERY_PIN);
            else if (strcmp(tEntry.IniKey, "reset_pin") == 0) tempValue = String(RESET_PIN);
            else if (strcmp(tEntry.IniKey, "setting_pin") == 0) tempValue = String(SETTING_PIN);
            else if (strcmp(tEntry.IniKey, "wake_pin") == 0) tempValue = String(SETTING_PIN);
            else if (strcmp(tEntry.IniKey, "display_width") == 0) tempValue = String(DISPLAY_WIDTH);
            else if (strcmp(tEntry.IniKey, "display_height") == 0) tempValue = String(DISPLAY_HEIGHT);
            else if (strcmp(tEntry.IniKey, "image_ext") == 0) tempValue = String(IMAGE_EXT);
            else if (strcmp(tEntry.IniKey, "images_dir") == 0) tempValue = String(IMAGES_DIR);
            else if (strcmp(tEntry.IniKey, "default_file_system") == 0) tempValue = (DEFAULT_FILE_SYSTEM == EFileSystemType::SDCard) ? "sdcard" : "littlefs";
            else if (strcmp(tEntry.IniKey, "fallback_enabled") == 0) tempValue = STORAGE_FALLBACK_ENABLED ? "true" : "false";
            break;
        }
      });
      strncpy(sValue, tempValue.c_str(), sizeof(sValue) - 1);
      sValue[sizeof(sValue) - 1] = '\0';
      return sValue;
    }
    return "";
  }

  bool Configuration_::SetConfig(const char *tKey, const char *tValue) {
    if (!tKey || tKey[0] == '\0' || !tValue) return false;
    String tLowerKey = tKey;
    tLowerKey.toLowerCase();
    bool tSuccess = false;
    const auto &tMapping = GetKeyMapping();
    auto tIt = std::find_if(tMapping.begin(), tMapping.end(), [&tLowerKey](const SConfigKeyMappingEntry &tEntry) {
      return tLowerKey.equals(tEntry.IniKey);
    });
    if (tIt != tMapping.end()) {
      const auto &tEntry = *tIt;
      if (tEntry.NvsKey[0] != '\0') {
        AccessConfig(false, [&]() {
          switch (tEntry.Type) {
            case EConfigType::BOOL:
              tSuccess = mConfig.putBool(tEntry.NvsKey, (strcasecmp(tValue, "true") == 0 || atoi(tValue) != 0));
              break;
            case EConfigType::UCHAR: {
              uint8_t tVal = static_cast<uint8_t>(atoi(tValue));
              if (strcmp(tEntry.NvsKey, kNvsTimerWakeHour) == 0) tVal %= 24;
              tSuccess = mConfig.putUChar(tEntry.NvsKey, tVal);
              break;
            }
            case EConfigType::USHORT:
              tSuccess = mConfig.putUShort(tEntry.NvsKey, static_cast<uint16_t>(atoi(tValue)));
              break;
            case EConfigType::INT:
              tSuccess = mConfig.putInt(tEntry.NvsKey, static_cast<int32_t>(atol(tValue)));
              break;
            case EConfigType::UINT:
            case EConfigType::ULONG:
              tSuccess = mConfig.putULong(tEntry.NvsKey, static_cast<uint32_t>(atol(tValue)));
              break;
            case EConfigType::STRING:
              tSuccess = mConfig.putString(tEntry.NvsKey, tValue);
              break;
            default:
              tSuccess = false;
              break;
          }
        });
      }
    }
    return tSuccess;
  }

  SAppConfig Configuration_::LoadConfigFromINI(const char *tFileName) {
    SAppConfig tConfig = Get<SAppConfig>();
    if (tFileName == nullptr || tFileName[0] == '\0') tFileName = CONFIG_FILE;
    tFileName = STG.NormalizePath(tFileName);
    if (!STG.Exists(tFileName)) {
      xLOG("INI file (%s) not found.", tFileName);
      xLOG("Using current NVS config.");
      return tConfig;
    }
    if (!ReadINIFile(tFileName, tConfig)) {
      xLOG("Failed to read INI file, using current config.");
      return tConfig;
    }
    xLOG("Configs loaded from file, ready to save.");
    SaveAllConfig(tConfig);
    return tConfig;
  }

  bool Configuration_::SaveAllConfig(const SAppConfig &tConfig) {
    bool tSuccess = true;
    AccessConfig(false, [&]() {
      tSuccess = tSuccess && mConfig.putString(kNvsDeviceAppName, tConfig.Device.Name);
      tSuccess = tSuccess && mConfig.putString(kNvsDeviceVersion, tConfig.Device.Version);
      tSuccess = tSuccess && mConfig.putUChar(kNvsDisplayBrightness, tConfig.Display.JpgBrightness.Get());
      tSuccess = tSuccess && mConfig.putUChar(kNvsDisplayContrast, tConfig.Display.JpgContrast.Get());
      tSuccess = tSuccess && mConfig.putUChar(kNvsDisplayGamma, tConfig.Display.JpgGamma.Get());
      if (tSuccess && tConfig.Display.CurrentFile.length() > 0) tSuccess = mConfig.putString(kNvsDisplayFile, tConfig.Display.CurrentFile);
      tSuccess = tSuccess && mConfig.putULong(kNvsDisplayImageUpdatedAt, tConfig.Display.ImageUpdatedAt);
      tSuccess = tSuccess && mConfig.putString(kNvsTimeServer, tConfig.Ntp.Server);
      tSuccess = tSuccess && mConfig.putUShort(kNvsTimePort, tConfig.Ntp.NtpPort.Get());
      tSuccess = tSuccess && mConfig.putInt(kNvsTimeGmtOffset, tConfig.Ntp.GMTOffset);
      tSuccess = tSuccess && mConfig.putInt(kNvsTimeGmtOffsetLong, tConfig.Ntp.GMTOffset);
      tSuccess = tSuccess && mConfig.putInt(kNvsTimeDaylightOffset, tConfig.Ntp.DaylightOffset);
      tSuccess = tSuccess && mConfig.putString(kNvsTimeZoneLabel, tConfig.Ntp.TimeZoneLabel);
      tSuccess = tSuccess && mConfig.putUInt(kNvsTimeUpdate, tConfig.Ntp.UpdateInterval);
      tSuccess = tSuccess && mConfig.putBool(kNvsTimeLowPowerSyncEnable, tConfig.Ntp.LowPowerSyncEnable);
      tSuccess = tSuccess && mConfig.putULong(kNvsTimeLowPowerSyncInterval, tConfig.Ntp.LowPowerSyncIntervalSec);
      tSuccess = tSuccess && mConfig.putULong(kNvsTimeLastSuccessfulSync, tConfig.Ntp.LastSuccessfulSyncEpochUtc);
      tSuccess = tSuccess && mConfig.putBool(kNvsConApEnable, tConfig.Connection.ApModeEnable);
      tSuccess = tSuccess && mConfig.putString(kNvsConApSsid, tConfig.Connection.ApSsid);
      tSuccess = tSuccess && mConfig.putString(kNvsConApPass, tConfig.Connection.ApPassword);
      tSuccess = tSuccess && mConfig.putString(kNvsConApIp, tConfig.Connection.ApIp);
      tSuccess = tSuccess && mConfig.putString(kNvsConApGw, tConfig.Connection.ApGateway);
      tSuccess = tSuccess && mConfig.putString(kNvsConApSubnet, tConfig.Connection.ApSubnet);
      tSuccess = tSuccess && mConfig.putString(kNvsConFallbackApSsid, tConfig.Connection.FallbackApSsid);
      tSuccess = tSuccess && mConfig.putString(kNvsConFallbackApPass, tConfig.Connection.FallbackApPassword);
      tSuccess = tSuccess && mConfig.putString(kNvsConFallbackApIp, tConfig.Connection.FallbackApIp);
      tSuccess = tSuccess && mConfig.putString(kNvsConFallbackApGw, tConfig.Connection.FallbackApGateway);
      tSuccess = tSuccess && mConfig.putString(kNvsConFallbackApSubnet, tConfig.Connection.FallbackApSubnet);
      tSuccess = tSuccess && mConfig.putString(kNvsConStaSsid, tConfig.Connection.StaSsid);
      tSuccess = tSuccess && mConfig.putString(kNvsConStaPass, tConfig.Connection.StaPassword);
      tSuccess = tSuccess && mConfig.putBool(kNvsConStaAutoFallback, tConfig.Connection.StaAutoFallbackApEnable);
      tSuccess = tSuccess && mConfig.putUChar(kNvsConStaMaxRetry, tConfig.Connection.StaConnectMaxRetry);
      tSuccess = tSuccess && mConfig.putUInt(kNvsConStaRetryDelayMs, tConfig.Connection.StaRetryDelayMs);
      tSuccess = tSuccess && mConfig.putBool(kNvsConStaEnable, tConfig.Connection.StaIpEnable);
      tSuccess = tSuccess && mConfig.putString(kNvsConStaIp, tConfig.Connection.StaIp);
      tSuccess = tSuccess && mConfig.putString(kNvsConStaGw, tConfig.Connection.StaGateway);
      tSuccess = tSuccess && mConfig.putString(kNvsConStaSubnet, tConfig.Connection.StaSubnet);
      tSuccess = tSuccess && mConfig.putString(kNvsConStaDns1, tConfig.Connection.StaPrimaryDns);
      tSuccess = tSuccess && mConfig.putString(kNvsConStaDns2, tConfig.Connection.StaSecondaryDns);
      tSuccess = tSuccess && mConfig.putBool(kNvsConMdnsEnable, tConfig.Connection.MdnsEnable);
      tSuccess = tSuccess && mConfig.putString(kNvsConMdnsName, tConfig.Connection.MdnsName);
      tSuccess = tSuccess && mConfig.putUChar(kNvsTimerWake, static_cast<uint8_t>(tConfig.Timer.WakeUp));
      tSuccess = tSuccess && mConfig.putUChar(kNvsTimerWakeHour, tConfig.Timer.WakeUpHour);
      tSuccess = tSuccess && mConfig.putBool(kNvsTelnetEnable, tConfig.Telnet.Enable);
      tSuccess = tSuccess && mConfig.putUChar(kNvsTelnetPort, tConfig.Telnet.TelnetPort.Get());
      tSuccess = tSuccess && mConfig.putString(kNvsTelnetUsername, tConfig.Telnet.Username);
      tSuccess = tSuccess && mConfig.putString(kNvsTelnetPassword, tConfig.Telnet.Password);
      tSuccess = tSuccess && mConfig.putULong(kNvsTelnetSession, tConfig.Telnet.Session);
      tSuccess = tSuccess && mConfig.putBool(kNvsFtpEnable, tConfig.Ftp.Enable);
      tSuccess = tSuccess && mConfig.putUChar(kNvsFtpPort, tConfig.Ftp.FtpPort.Get());
      tSuccess = tSuccess && mConfig.putString(kNvsFtpUsername, tConfig.Ftp.Username);
      tSuccess = tSuccess && mConfig.putString(kNvsFtpPassword, tConfig.Ftp.Password);
    });
    if (tSuccess) xLOG("Config saved successfully!");
    else xLOG("Failed save, some values may not have been written!");
    return tSuccess;
  }

} 
