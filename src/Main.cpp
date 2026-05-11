#include <App/Global.h>

namespace App {
  RTC_DATA_ATTR uint32_t gBootCount = 0;
}

using namespace App;

SET_LOOP_TASK_STACK_SIZE(LOOP_TASK_STACK_SIZE);

class Application {

  DEFINE_TAG("APP");
  friend class AutoGuard<Application>;
  static bool sButtonTaskStarted;

  public:

    using Guard = AutoGuard<Application>;

    static Application &Instance() {
      static Application tInstance;
      return tInstance;
    };

    void Init() {     
      #if !PRODUCTION
        xLOG_B(BAUDRATE);
        unsigned long tStart = millis();
        while (!xLOG_S && (millis() - tStart) < 5e3) vTaskDelay(10 / portTICK_PERIOD_MS);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        xLOG_PL();
        xLOG_FLUSH();
        {
          Guard tLock;
          UTL.PrintPartitionInfo();
        }
      #endif
      if (psramFound()) heap_caps_malloc_extmem_enable(128);
      if (!mMutex) mMutex = xSemaphoreCreateRecursiveMutex();
      gBootCount++;
      if(!CFG.Init()) return;
      ReloadConfig();
      UTL.Init();
      #if PRODUCTION
        if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
          const esp_sleep_wakeup_cause_t tWakeCause = esp_sleep_get_wakeup_cause();
          const bool tAllowedWakeCause = (tWakeCause == ESP_SLEEP_WAKEUP_TIMER) || (tWakeCause == ESP_SLEEP_WAKEUP_EXT1) || (tWakeCause == ESP_SLEEP_WAKEUP_EXT0);
          if (!tAllowedWakeCause) {
            UTL.SleepAndWakeup();
            return;
          }
        }
      #endif
      UTL.SetCPUFrequency(ECPUFrequency::F160MHz);
      #if !PRODUCTION
        { 
          Guard tLock; 
          UTL.PrintDeviceInfo(); 
        }
      #endif
      UTL.DisableBrownout();
      UTL.DisableBT();
      UTL.DisableTouchPad();
      if (RTC.Init() && RTC.IsAvailable()) {
        RTC.SyncToSystem();
        RTC.End();
        UTL.PrintDateTime();
      }
      if (!UTL.MeasureBattery()) {
        LowBatteryMode();
        return;
      }
      #if !PRODUCTION
        BTN.AddPin(mCfg.Device.SettingPin, "[settings button]");
        BTN.AddPin(mCfg.Device.ResetPin, "[reset button]");
        BTN.AddShortPressCallback(mCfg.Device.SettingPin, []() {
          Instance().MaintenanceMode();
        });
        if (!sButtonTaskStarted) {
          BTN.Start();
          xTaskCreatePinnedToCore(&ButtonTask, "ButtonTask", BUTTON_TASK_STACK_SIZE, nullptr, 12, nullptr, 1);
          sButtonTaskStarted = true;
        }
      #endif
      FWU.CleanupUpdateDirOnBoot();
      if (UTL.WasWokenByButton()) MaintenanceMode();
      else PhotoFrameMode();
    }

    void Run() {
      vTaskDelay(portMAX_DELAY);
    }

  private:
    Application() = default;
    SemaphoreHandle_t mMutex = nullptr;
    SAppConfig mCfg {};
    uint32_t mMaintenanceLastActivityMs = 0;
    bool mExitMaintenanceMode = false;

    static void Lock() {
      if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
    }

    static void Unlock() {
      if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
    }

    void ReloadConfig() {
      Guard tLock;
      mCfg = CFG.Get<SAppConfig>();
    }

    void ShowDefaultImage() {
      DSP.PrintImage(0, 0, DefaultImageWidth, DefaultImageHeight, DefaultImage);
      DSP.ClearDisplay();
      DSP.Update();
    }

    void SaveNextImage(const char *tNextImage) {
      if (!CFG.SaveImageName(tNextImage)) xLOG("Failed to save next image name");
      else xLOG("Next image → %s", tNextImage);
    }

    bool TryDisplayImage(const char *tImage) {
      if (!tImage || *tImage == '\0') return false;
      char tFullPath[128] = "";
      snprintf(tFullPath, sizeof(tFullPath), "/%s/%s", mCfg.Display.ImagesDir.c_str(), tImage);
      if (!STG.Exists(tFullPath)) return false;
      xLOG("Trying image → %s", tImage);
      return DSP.PrintJpg(0, 0, tImage);
    }

    bool WaitForWifiClient(uint32_t tTimeoutMs) {
      const uint32_t tStartMs = millis();
      while (!CON.HasActiveWifiClient()) {
        if (UTL.HasElapsedMs(tStartMs, millis(), tTimeoutMs)) return false;
        vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
      }
      return true;
    }

    void TouchMaintenanceActivity() {
      mMaintenanceLastActivityMs = millis();
    }

    void PhotoFrameMode() {
      ReloadConfig();
      UTL.PrintInfo("Device starts in Photo Frame Mode", EUtilsInfoType::Single);
      STG.Init(true);
      LOG.Init();
      LOG.Boot(UTL.ResolveBootReason(), "PHOTO_FRAME", mCfg.Device.Version.c_str(), gBootCount);
      LOG.Battery(UTL.mBatteryPercentage, static_cast<uint16_t>(UTL.mBatteryVoltage * 1000.0f), "measured");
      DSP.Init();
      const char *tImage = mCfg.Display.CurrentFile.isEmpty() ? STG.GetNextFile("")  : mCfg.Display.CurrentFile.c_str();
      if (TryDisplayImage(tImage)) SaveNextImage(STG.GetNextFile(tImage));
      else {
        xLOG("Image failed → %s", tImage ? tImage : "(null)");
        const char *tNextImage = STG.GetNextFile(tImage ? tImage : "");
        if (TryDisplayImage(tNextImage)) SaveNextImage(STG.GetNextFile(tNextImage));
        else {
          xLOG("Displaying default image.");
          ShowDefaultImage();
          if (!CFG.SaveImageName("")) xLOG("Failed to clear image name.");
          else xLOG("Image name cleared.");
        }
      }
      UTL.PrintMemoryInfo();
      DSP.OffAll();
      LOG.Halt("SLEEP");
      STG.End();
      #if PRODUCTION
        UTL.SleepAndWakeup();
        __builtin_unreachable();
      #else
        while (true) vTaskDelay(1e3 / portTICK_PERIOD_MS);
      #endif
    }

    void MaintenanceMode() {
      UTL.SetCPUFrequency(ECPUFrequency::F240MHz);
      ReloadConfig();
      {
        char tText[45] = "";
        snprintf(tText, sizeof(tText), "Device starts in Maintenance [%s] Mode", (mCfg.Connection.ApModeEnable ? "AP" : "STA"));
        UTL.PrintInfo(tText, EUtilsInfoType::Single);
      }
      TON.Init(TONE_PIN);
      TON.Play(kMaintenanceTone);
      STG.Init(true);
      LOG.Init();
      LOG.Boot(UTL.ResolveBootReason(), "MAINTENANCE", mCfg.Device.Version.c_str(), gBootCount);
      LOG.Battery(UTL.mBatteryPercentage, static_cast<uint16_t>(UTL.mBatteryVoltage * 1000.0f), "measured");
      TouchMaintenanceActivity();
      DSP.Init();
      CON.Init(true);
      vTaskDelay(DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
      xLOG_FLUSH();
      if (!sButtonTaskStarted) {
        BTN.AddPin(mCfg.Device.SettingPin, "[settings button]");
        BTN.AddPin(mCfg.Device.ResetPin, "[reset button]");
        BTN.Start();
        xTaskCreatePinnedToCore(&ButtonTask, "ButtonTask", BUTTON_TASK_STACK_SIZE, nullptr, 12, nullptr, 1);
        sButtonTaskStarted = true;
      }
      BTN.AddLongPressCallback(mCfg.Device.SettingPin, []() {
        #if !PRODUCTION
          xLOG("Exiting Maintenance Mode, entering Photo Frame Mode...");
          vTaskDelay(DELAY_SHORT_MS / portTICK_PERIOD_MS);
        #endif
        Instance().mExitMaintenanceMode = true;
      }, REBOOT_LONG_PRESS_MS);
      BTN.AddLongPressCallback(mCfg.Device.ResetPin, []() {
        #if !PRODUCTION
          xLOG("Device factory reset...");
          vTaskDelay(DELAY_SHORT_MS / portTICK_PERIOD_MS);
        #endif
        { 
          Guard tLock;
          LOG.Halt("FACTORY_RESET");
          CFG.FactoryReset();
          DSP.OffAll();
          STG.End();
          CON.Stop();
          if (Instance().mCfg.Ftp.Enable) FTP.End();
          if (Instance().mCfg.Telnet.Enable) {
            TLN.ClearSession();
            TLN.mWaitingPassword = false;
            TLN.mExitRequested = true;
          }
        }
        esp_restart();
      }, FACTORY_RESET_LONG_PRESS_MS);
      vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
      char tTitleBuffer[64];
      DSP.SetFont(&OpenSans13B);
      snprintf(tTitleBuffer, sizeof(tTitleBuffer), "%s ¤ MAINTENANCE [%s MODE]", mCfg.Device.Name.c_str(), (mCfg.Connection.ApModeEnable ? "AP" : "STA"));
      DSP.WriteTextWithBoxCentered(tTitleBuffer);     
      if (mCfg.Connection.ApModeEnable) { 
        DSP.SetFont(&OpenSans11);
        DSP.SetColor(EDisplayColor::Gray);
        snprintf(tTitleBuffer, sizeof(tTitleBuffer), "Connect to Wifi AP Mode to SSID: %s", mCfg.Connection.ApSsid.c_str());
        DSP.WriteText(0, 310, tTitleBuffer, EDisplayHAlignment::Center);
        DSP.ClearDisplay();
        DSP.Update();
        if (WaitForWifiClient(WIFI_CONNECT_TIMEOUT_MS)) TouchMaintenanceActivity();
      }
      DSP.ClearDisplay();
      DSP.Update();
      if (WaitForWifiClient(WIFI_CONNECT_TIMEOUT_MS)) TouchMaintenanceActivity();
      if (mCfg.Telnet.Enable) {
        TLN.ActivityCallback([this]() { TouchMaintenanceActivity(); });
        TLN.Init(true);
        vTaskDelay(DELAY_HALF_SEC_MS / portTICK_PERIOD_MS);
      }
      if (mCfg.Ftp.Enable) {
        FTP.EventCallback([this](FtpOperation, uint32_t, uint32_t) {
          TouchMaintenanceActivity();
        });
        FTP.TransferCallback([this](FtpTransferOperation, const char *, uint32_t) {
          TouchMaintenanceActivity();
        });
        FTP.Init(true);
        FTP.Callback([this](const char *tFileName, uint32_t tFileSize) {
          Guard tLock;
          TouchMaintenanceActivity();
          mCfg = CFG.Get<SAppConfig>();
          const char *tConfigFile = mCfg.Device.ConfigFile.c_str();
          tConfigFile = LFS.GetFileName(tConfigFile);
          if (tFileName && strcmp(tFileName, tConfigFile) == 0) mCfg = CFG.LoadConfigFromINI(tConfigFile);
        });
      }
      if (mCfg.Connection.ApModeEnable) DSP.FillRect(0, 300, mCfg.Display.Width, 50, EDisplayColor::White);
      DSP.SetFont(&OpenSans11);
      DSP.SetColor(EDisplayColor::Gray);
      size_t tPos = 0;
      if (mCfg.Telnet.Enable) tPos += snprintf(tTitleBuffer + tPos, sizeof(tTitleBuffer) - tPos, "telnet %s %d", CON.GetIpAddress(), mCfg.Telnet.TelnetPort.Get());
      if (mCfg.Ftp.Enable) {
        if (tPos > 0) tPos += snprintf(tTitleBuffer + tPos, sizeof(tTitleBuffer) - tPos, " ¤ ");
        tPos += snprintf(tTitleBuffer + tPos, sizeof(tTitleBuffer) - tPos, "ftp %s %d", CON.GetIpAddress(), mCfg.Ftp.FtpPort.Get());
      }
      if (tPos > 0) DSP.WriteText(0, 310, tTitleBuffer, EDisplayHAlignment::Center);
      if ((mCfg.Telnet.Enable || mCfg.Ftp.Enable) && mCfg.Connection.MdnsEnable) {
        snprintf(tTitleBuffer, sizeof(tTitleBuffer), "localhost %s.local", mCfg.Connection.MdnsName.c_str());
        DSP.WriteText(0, 340, tTitleBuffer, EDisplayHAlignment::Center);
      }
      DSP.ClearDisplay();
      DSP.Update();
      if (mCfg.Telnet.Enable) xTaskCreatePinnedToCore(&TelnetTask, "TelnetTask", TELNET_TASK_STACK_SIZE, nullptr, 10, nullptr, 1);
      if (mCfg.Ftp.Enable) xTaskCreatePinnedToCore(&FTPTask, "FTPTask", FTP_TASK_STACK_SIZE, nullptr, 11, nullptr, 1);
      UTL.PrintMemoryInfo();
      while (true) {
        vTaskDelay(DELAY_ONE_SEC_MS / portTICK_PERIOD_MS);
        if (mExitMaintenanceMode) {
          #if !PRODUCTION
            xLOG("Exiting Maintenance Mode...");
          #endif
          {
            Guard tLock;
            LOG.Halt("EXIT_MAINTENANCE");
            DSP.OffAll();
            STG.End();
            CON.Stop();
            if (mCfg.Ftp.Enable) FTP.End();
            if (mCfg.Telnet.Enable) TLN.End();
          }
          mExitMaintenanceMode = false;
          PhotoFrameMode();
          return;
        }
        if (!UTL.HasElapsedMs(mMaintenanceLastActivityMs, millis(), MAINTENANCE_INACTIVITY_TIMEOUT_MS)) continue;
        {
          Guard tLock;
          LOG.Halt("INACTIVITY_TIMEOUT");
          DSP.OffAll();
          STG.End();
          CON.Stop();
        }
        esp_restart();
        __builtin_unreachable();
      }
    }

    void LowBatteryMode() {
      UTL.PrintInfo("Device starts in Low Battery Mode", EUtilsInfoType::Single);
      TON.Init(TONE_PIN);
      TON.Play(kLowBatteryTone);
      LFS.Init(true);
      DSP.Init();
      char tBuffer[32];
      const int32_t tWidth = 160;
      const int32_t tHeight = 60;
      const int32_t tLayers = 2;
      const int32_t tLevelPadding = 5;
      const int32_t tStartX = (mCfg.Display.Width - tWidth) / 2;
      const int32_t tStartY = (mCfg.Display.Height - tHeight) / 2;
      const int32_t tButtonWidth = tWidth / 10 + tLayers;
      const int32_t tButtonHeight = tHeight / 2;
      const int32_t tBodyWidth = tWidth - tButtonWidth;
      const int32_t tBodyHeight = tHeight;     
      for (int32_t tLayer = 0; tLayer < tLayers; tLayer++) {
        int32_t tOffset = tLayer;
        int32_t tBtnX = tStartX + tOffset;
        int32_t tBtnY = tStartY + tOffset + (tHeight - tButtonHeight) / 2;
        int32_t tBtnW = tButtonWidth - 2 * tLayer + tLayers;
        int32_t tBtnH = tButtonHeight - 2 * tLayer;
        DSP.DrawRect(tBtnX, tBtnY, tBtnW, tBtnH, EDisplayColor::Black);
        int32_t tBodyX = tStartX + tOffset + tButtonWidth;
        int32_t tBodyY = tStartY + tOffset;
        int32_t tBodyW = tBodyWidth - 2 * tLayer;
        int32_t tBodyH = tBodyHeight - 2 * tLayer;
        DSP.DrawRect(tBodyX, tBodyY, tBodyW, tBodyH, EDisplayColor::Black);
      }
      int32_t tLevelW = (tBodyWidth - 2 * tLayers - 6 * tLevelPadding) / 5;
      int32_t tLevelH = tBodyHeight - 2 * tLayers - 2 * tLevelPadding;
      int32_t tLevelX = tStartX +  tButtonWidth + tLevelPadding + (tLevelW + tLevelPadding) * 4;
      int32_t tLevelY = tStartY + tLayers + tLevelPadding;
      DSP.FillRect(tLevelX, tLevelY, tLevelW, tLevelH, EDisplayColor::LightGray);
      DSP.SetFont(&OpenSans11);
      DSP.SetColor(EDisplayColor::Gray);
      snprintf(tBuffer, sizeof(tBuffer), "low battery: %.2fV [%d%%]", UTL.mBatteryVoltage, UTL.mBatteryPercentage);
      DSP.WriteText(0, tStartY + tHeight + 25, tBuffer, EDisplayHAlignment::Center, EDisplayVAlignment::Auto, EDisplayColor::White);
      DSP.ClearDisplay();
      DSP.Update();
      DSP.OffAll();
      UTL.PrintMemoryInfo();
      LFS.End();
      #if PRODUCTION
        UTL.SleepLowBattery();
        __builtin_unreachable();
      #else
        while (true) vTaskDelay(1e3 / portTICK_PERIOD_MS);
      #endif     
    }

    static void ButtonTask(void *tParameter) {
      while (true) {
        BTN.HandleEvents();
        vTaskDelay(DELAY_ULTRA_SHORT_MS / portTICK_PERIOD_MS);
      }
    }

    static void TelnetTask(void *tParameter) {
      while (true) {
        if (CON.HasActiveWifiClient()) TLN.HandleEvents();
        vTaskDelay(DELAY_ULTRA_SHORT_MS / portTICK_PERIOD_MS);
      }
    }  

    static void FTPTask(void *tParameter) {
      while (true) {
        if (CON.HasActiveWifiClient()) FTP.HandleEvents();
        vTaskDelay(DELAY_ULTRA_SHORT_MS / portTICK_PERIOD_MS);
      }
    }
    
};

bool Application::sButtonTaskStarted = false;

#define APP Application::Instance()

void setup() {
  APP.Init();
};

void loop() {
  APP.Run();
};

