#include <App/Connection.h>

namespace App {

  WiFiClass *Connection_::mWiFi = nullptr;
  TaskHandle_t Connection_::mTaskHandle = nullptr;
  volatile bool Connection_::sStopWiFiEventTask = false;
  
  Connection_ &Connection_::Instance() {
    static Connection_ tInstance;
    return tInstance;
  }

  Connection_::Connection_() {
    mMutex = xSemaphoreCreateRecursiveMutex();
    if (!mWiFi) mWiFi = new WiFiClass();
  }

  Connection_::~Connection_() {
    delete mWiFi;
    mWiFi = nullptr;
    if (mMutex) {
      vSemaphoreDelete(mMutex);
      mMutex = nullptr;
    }
  }

  void Connection_::Lock() {
    if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
  }

  void Connection_::Unlock() {
    if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
  }

  void Connection_::Init(bool tVerbose) {
    ReloadConfig();
    Start();
    if (mCfg.Connection.ApModeEnable) {
      vTaskDelay(DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
      return;
    }
    uint32_t tStart = millis();
    while (true) {
      bool tWifiActive = HasActiveWifiClient();
      if (tWifiActive) {
        BootstrapVault();
        break;
      }
      if (millis() - tStart > WIFI_CONNECT_TIMEOUT_MS) {
        xLOG("Connection timeout, timedate syncronization skipped!");
        break;
      }
      vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
    }
  }

  void Connection_::ReloadConfig() {
    Guard tLock;
    mCfg = CFG.Get<SAppConfig>();
  }

  void Connection_::Start() {
    sStopWiFiEventTask = false;
    if (!mTaskHandle) xTaskCreatePinnedToCore(&WiFiEventTask, "WiFiEventTask", kConTaskStack, this, 1, &mTaskHandle, 1);
  }

  void Connection_::Stop() {
    sStopWiFiEventTask = true;
    if (mTaskHandle) {
      for (uint8_t i = 0; i < 20 && eTaskGetState(mTaskHandle) != eDeleted; ++i) vTaskDelay(DELAY_SHORT_MS / portTICK_PERIOD_MS);
      mTaskHandle = nullptr;
    }
    Guard tLock;
    if (mWiFi) mWiFi->mode(WIFI_OFF);
  }

  const char *Connection_::GetIpAddress() {
    static char sIpBuffer[16] = "0.0.0.0";
    Guard tLock;
    if (mWiFi) {
      if (mWiFi->getMode() & WIFI_AP) {
        IPAddress tIp = mWiFi->softAPIP();
        snprintf(sIpBuffer, sizeof(sIpBuffer), "%d.%d.%d.%d", tIp[0], tIp[1], tIp[2], tIp[3]);
      } else if (mWiFi->getMode() & WIFI_STA && mWiFi->status() == WL_CONNECTED) {
        IPAddress tIp = mWiFi->localIP();
        snprintf(sIpBuffer, sizeof(sIpBuffer), "%d.%d.%d.%d", tIp[0], tIp[1], tIp[2], tIp[3]);
      }
    }
    return sIpBuffer;
  }

  bool Connection_::HasActiveWifiClient() const {
    bool tActive = false;
    Guard tLock;
    if (!mWiFi) return false;
    wifi_mode_t tMode = mWiFi->getMode();
    if (tMode & WIFI_AP) tActive = mWiFi->softAPgetStationNum() > 0;
    else if (tMode & WIFI_STA) tActive = mWiFi->status() == WL_CONNECTED;
    return tActive;
  }

  void Connection_::Callback(FConnectionCallback tCallback) {
    Guard tLock;
    mCallback = tCallback;
  }

  void Connection_::WiFiEventTask(void *tParameter) {
    Connection_ *tSelf = static_cast<Connection_ *>(tParameter);
    bool tLastActive = false;
    bool tMdnsRunning = false;
    while (!sStopWiFiEventTask) {
      bool tCurrentActive = false;
      if (tSelf->mCfg.Connection.ApModeEnable) {
        tSelf->SetupAp();
        tCurrentActive = (mWiFi->softAPgetStationNum() > 0);
      } else {
        bool tStaConnected = tSelf->TryConnectStaWithRetry();
        if (tStaConnected) {
          tCurrentActive = true;
        } else if (tSelf->mCfg.Connection.StaAutoFallbackApEnable) {
          xLOG("Trying AP+STA maintenance mode");
          if (!tSelf->TryConnectApSta()) {
            xLOG("Switching to fallback AP");
            tSelf->SwitchToFallbackApMode();
          }
          tCurrentActive = true;
        } else {
          xLOG("WiFi offline");
          tCurrentActive = false;
        }
      }
      if (tCurrentActive && !tLastActive) {
        if (tSelf->mCfg.Connection.ApModeEnable) {
          xLOG("Client connected to AP clients → %d", mWiFi->softAPgetStationNum());
        }
        if (tSelf->mCfg.Connection.MdnsEnable) {
          tSelf->StartMdns();
          tMdnsRunning = true;
        }
        if (tSelf->mCallback) tSelf->mCallback();
      }
      if (!tCurrentActive && tLastActive) {
        if (tSelf->mCfg.Connection.ApModeEnable) {
          xLOG("All clients disconnected from AP");
        }
        if (tMdnsRunning) {
          tSelf->mMDNS.end();
          tMdnsRunning = false;
          xLOG("Localhost stopped, connection lost");
        }
      }
      tLastActive = tCurrentActive;
      vTaskDelay(tSelf->mCfg.Connection.ApModeEnable ? DELAY_SHORT_MS / portTICK_PERIOD_MS : DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
    }
    vTaskDelete(nullptr);
  }

  void Connection_::SetupAp() {
    if (mWiFi->getMode() == WIFI_AP && mWiFi->softAPSSID() == mCfg.Connection.ApSsid) return;
    IPAddress tIp, tGateway, tSubnet;
    tIp.fromString(mCfg.Connection.ApIp.c_str());
    tGateway.fromString(mCfg.Connection.ApGateway.c_str());
    tSubnet.fromString(mCfg.Connection.ApSubnet.c_str());
    mWiFi->mode(WIFI_AP);
    mWiFi->persistent(false);
    mWiFi->softAPConfig(tIp, tGateway, tSubnet);
    mWiFi->softAP(mCfg.Connection.ApSsid.c_str(), mCfg.Connection.ApPassword.c_str());
    xLOG("Starting AP Mode → %s", mCfg.Connection.ApSsid.c_str());
    PrintConnectionInfo();
  }

  void Connection_::ConnectSta() {
    if (mWiFi->status() == WL_CONNECTED || (mWiFi->getMode() == WIFI_STA && mWiFi->status() == WL_IDLE_STATUS)) return;
    mWiFi->mode(WIFI_STA);
    mWiFi->useStaticBuffers(true);
    if (mCfg.Connection.StaIpEnable) {
      IPAddress tIp, tGateway, tSubnet, tDns1, tDns2;
      tIp.fromString(mCfg.Connection.StaIp.c_str());
      tGateway.fromString(mCfg.Connection.StaGateway.c_str());
      tSubnet.fromString(mCfg.Connection.StaSubnet.c_str());
      tDns1.fromString(mCfg.Connection.StaPrimaryDns.c_str());
      tDns2.fromString(mCfg.Connection.StaSecondaryDns.c_str());
      mWiFi->config(tIp, tGateway, tSubnet, tDns1, tDns2);
    }
    mWiFi->hostname(mCfg.Connection.MdnsName);
    xLOG("Connecting to WiFi → %s", mCfg.Connection.StaSsid.c_str());
    mWiFi->begin(mCfg.Connection.StaSsid.c_str(), mCfg.Connection.StaPassword.c_str());
    PrintConnectionInfo();
  }

  bool Connection_::TryConnectStaWithRetry() {
    uint8_t tMaxRetry = mCfg.Connection.StaConnectMaxRetry;
    if (tMaxRetry == 0) tMaxRetry = 1;
    uint32_t tRetryDelayMs = mCfg.Connection.StaRetryDelayMs;
    xLOG("STA connect, max retries=%u, delay=%lums", tMaxRetry, tRetryDelayMs);
    for (uint8_t tAttempt = 0; tAttempt < tMaxRetry; tAttempt++) {
      ConnectSta();
      uint32_t tStart = millis();
      while (mWiFi->status() != WL_CONNECTED && (millis() - tStart) < WIFI_CONNECT_TIMEOUT_MS) {
        vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
      }
      if (mWiFi->status() == WL_CONNECTED) {
        xLOG("STA connected successfully @ attempt %u/%u", tAttempt + 1, tMaxRetry);
        return true;
      }
      if (tAttempt < tMaxRetry - 1) {
        xLOG("STA attempt %u/%u failed, retrying in %lums...", tAttempt + 1, tMaxRetry, tRetryDelayMs);
        vTaskDelay(pdMS_TO_TICKS(tRetryDelayMs));
      }
    }
    xLOG("STA connect failed after %u attempts, consider fallback to AP", tMaxRetry);
    return false;
  }

  bool Connection_::TryConnectApSta() {
    String tApSsid = mCfg.Connection.FallbackApSsid.length() ? mCfg.Connection.FallbackApSsid : mCfg.Connection.ApSsid;
    String tApPassword = mCfg.Connection.FallbackApPassword.length() ? mCfg.Connection.FallbackApPassword : mCfg.Connection.ApPassword;
    String tApIpStr = mCfg.Connection.FallbackApIp.length() ? mCfg.Connection.FallbackApIp : mCfg.Connection.ApIp;
    String tApGatewayStr = mCfg.Connection.FallbackApGateway.length() ? mCfg.Connection.FallbackApGateway : mCfg.Connection.ApGateway;
    String tApSubnetStr = mCfg.Connection.FallbackApSubnet.length() ? mCfg.Connection.FallbackApSubnet : mCfg.Connection.ApSubnet;

    IPAddress tApIp, tApGateway, tApSubnet;
    tApIp.fromString(tApIpStr.c_str());
    tApGateway.fromString(tApGatewayStr.c_str());
    tApSubnet.fromString(tApSubnetStr.c_str());

    mWiFi->mode(WIFI_AP_STA);
    mWiFi->persistent(false);
    mWiFi->softAPConfig(tApIp, tApGateway, tApSubnet);
    bool tApStarted = mWiFi->softAP(tApSsid.c_str(), tApPassword.c_str());
    if (!tApStarted) {
      xLOG("AP start failed");
      return false;
    }

    if (mCfg.Connection.StaIpEnable) {
      IPAddress tIp, tGateway, tSubnet, tDns1, tDns2;
      tIp.fromString(mCfg.Connection.StaIp.c_str());
      tGateway.fromString(mCfg.Connection.StaGateway.c_str());
      tSubnet.fromString(mCfg.Connection.StaSubnet.c_str());
      tDns1.fromString(mCfg.Connection.StaPrimaryDns.c_str());
      tDns2.fromString(mCfg.Connection.StaSecondaryDns.c_str());
      mWiFi->config(tIp, tGateway, tSubnet, tDns1, tDns2);
    }
    mWiFi->hostname(mCfg.Connection.MdnsName);

    if (mCfg.Connection.StaSsid.length()) {
      mWiFi->begin(mCfg.Connection.StaSsid.c_str(), mCfg.Connection.StaPassword.c_str());
      uint32_t tStart = millis();
      while (mWiFi->status() != WL_CONNECTED && (millis() - tStart) < WIFI_CONNECT_TIMEOUT_MS) {
        vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
      }
      if (mWiFi->status() == WL_CONNECTED) xLOG("STA connected while AP is active");
      else xLOG("STA not connected, AP remains active");
    } else xLOG("STA SSID empty, AP-only fallback behavior");

    PrintConnectionInfo();
    return true;
  }

  void Connection_::SwitchToFallbackApMode() {
    if (mCfg.Connection.FallbackApSsid.length() == 0) {
      xLOG("Fallback AP SSID not configured, using default AP settings");
      SetupAp();
      return;
    }
    xLOG("Switching to fallback AP mode → %s", mCfg.Connection.FallbackApSsid.c_str());
    IPAddress tIp, tGateway, tSubnet;
    tIp.fromString(mCfg.Connection.FallbackApIp.c_str());
    tGateway.fromString(mCfg.Connection.FallbackApGateway.c_str());
    tSubnet.fromString(mCfg.Connection.FallbackApSubnet.c_str());
    mWiFi->mode(WIFI_AP);
    mWiFi->persistent(false);
    mWiFi->softAPConfig(tIp, tGateway, tSubnet);
    mWiFi->softAP(mCfg.Connection.FallbackApSsid.c_str(), mCfg.Connection.FallbackApPassword.c_str());
    xLOG("Fallback AP activated → %s @ %s", mCfg.Connection.FallbackApSsid.c_str(), mCfg.Connection.FallbackApIp.c_str());
    PrintConnectionInfo();
  }

  void Connection_::StartMdns() {
    mMDNS.end();
    vTaskDelay(DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
    uint8_t tRetry = 0;
    while (!(mWiFi->status() == WL_CONNECTED || mCfg.Connection.ApModeEnable) && tRetry++ < 10) vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
    if (!(mWiFi->status() == WL_CONNECTED || mCfg.Connection.ApModeEnable)) {
      xLOG("Localhost start failed, no active interface");
      return;
    }
    bool tStarted = false;
    for (uint8_t i = 0; i < 3 && !tStarted; ++i) {
      if (i > 0) {
        mMDNS.end();
        vTaskDelay(DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
      }
      tStarted = mMDNS.begin(mCfg.Connection.MdnsName);
      if (!tStarted) {
        xLOG("Localhost start, attempt %d failed", i + 1);
        vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
      }
    }
    if (tStarted) {
      if (mCfg.Ftp.Enable) mMDNS.addService("ftp", "tcp", mCfg.Ftp.FtpPort);
      if (mCfg.Telnet.Enable) mMDNS.addService("telnet", "tcp", mCfg.Telnet.TelnetPort);
      xLOG("Localhost started → %s.local", mCfg.Connection.MdnsName.c_str());
    } else xLOG("Localhost failed to start after 3 attempts");
  }

  void Connection_::PrintConnectionInfo() {
    char tText[UTL.GetPrintInfoWidth() - 4] = "";
    xLOG_PL();
    if (mCfg.Connection.ApModeEnable) {
      UTL.PrintInfo("CONNECTION: AP MODE", EUtilsInfoType::Header);
      UTL.PrintInfo("", EUtilsInfoType::Line);
      snprintf(tText, sizeof(tText), "SSID: %s", mCfg.Connection.ApSsid.c_str());
      UTL.PrintInfo(tText);
      UTL.PrintInfo("", EUtilsInfoType::Line);
      snprintf(tText, sizeof(tText), "IP: %s", mWiFi->softAPIP().toString().c_str());
      UTL.PrintInfo(tText);
      snprintf(tText, sizeof(tText), "Gateway: %s", mCfg.Connection.ApGateway.c_str());
      UTL.PrintInfo(tText);
      snprintf(tText, sizeof(tText), "Subnet: %s", mWiFi->softAPSubnetMask().toString().c_str());
      UTL.PrintInfo(tText);
      UTL.PrintInfo("", EUtilsInfoType::Line);
      snprintf(tText, sizeof(tText), "MAC: %s", mWiFi->macAddress().c_str());
      UTL.PrintInfo(tText);
      UTL.PrintInfo("", EUtilsInfoType::Footer);
      xLOG("Connect to WiFi AP: %s", mCfg.Connection.ApSsid.c_str());
    } else {
      snprintf(tText, sizeof(tText), "CONNECTION: STA MODE %s", mCfg.Connection.StaIpEnable ? "(STATIC)" : "(DHCP)");
      UTL.PrintInfo(tText, EUtilsInfoType::Header);
      UTL.PrintInfo("", EUtilsInfoType::Line);
      snprintf(tText, sizeof(tText), "SSID: %s", mWiFi->SSID().c_str());
      UTL.PrintInfo(tText);
      UTL.PrintInfo("", EUtilsInfoType::Line);
      snprintf(tText, sizeof(tText), "IP: %s", mWiFi->localIP().toString().c_str());
      UTL.PrintInfo(tText);
      snprintf(tText, sizeof(tText), "Gateway: %s", mWiFi->gatewayIP().toString().c_str());
      UTL.PrintInfo(tText);
      snprintf(tText, sizeof(tText), "Subnet: %s", mWiFi->subnetMask().toString().c_str());
      UTL.PrintInfo(tText);
      snprintf(tText, sizeof(tText), "DNS 1: %s", mWiFi->dnsIP(0).toString().c_str());
      UTL.PrintInfo(tText);
      snprintf(tText, sizeof(tText), "DNS 2: %s", mWiFi->dnsIP(1).toString().c_str());
      UTL.PrintInfo(tText);      
      UTL.PrintInfo("", EUtilsInfoType::Line);
      snprintf(tText, sizeof(tText), "MAC: %s", mWiFi->macAddress().c_str());
      UTL.PrintInfo(tText);
      UTL.PrintInfo("", EUtilsInfoType::Footer);
    }
  }

  void Connection_::BootstrapVault() {
    Guard tLock;
    NTP.Init();
    NTP.SyncSystemTimeIfNeeded();
    NTP.PrintDateTimeInfo();
    NTP.End();
  }

  bool Connection_::SyncTimeIfDue() {
    ReloadConfig();
    if (mCfg.Connection.ApModeEnable) {
      xLOG("AP mode active, low-power NTP sync skipped");
      return false;
    }
    if (mCfg.Ntp.LowPowerSyncEnable) {
      unsigned long tCurrentEpoch = static_cast<unsigned long>(time(nullptr));
      unsigned long tLastSync = mCfg.Ntp.LastSuccessfulSyncEpochUtc;
      if (tLastSync > 0 && tCurrentEpoch >= tLastSync) {
        unsigned long tElapsed = tCurrentEpoch - tLastSync;
        if (tElapsed < mCfg.Ntp.LowPowerSyncIntervalSec) {
          xLOG("NTP sync not due (synced %lu sec ago, interval %lu sec)", tElapsed, mCfg.Ntp.LowPowerSyncIntervalSec);
          return false;
        }
      }
    }
    xLOG("Low-power NTP sync starting...");
    Start();
    uint32_t tStart = millis();
    while (!HasActiveWifiClient()) {
      if (millis() - tStart > WIFI_CONNECT_TIMEOUT_MS) {
        xLOG("WiFi connect timeout, NTP sync skipped");
        Stop();
        return false;
      }
      vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
    }
    BootstrapVault();
    Stop();
    return true;
  }

}
