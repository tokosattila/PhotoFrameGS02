#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <App/Global.h>

namespace App {

  enum class EConfigType {
    BOOL = 1, 
    UCHAR, 
    INT, 
    UINT, 
    ULONG, 
    USHORT, 
    STRING, 
    GLOBAL_INT
  };

  struct SConfigKeyMappingEntry {
    const char *NvsKey;
    const char *IniKey;
    const char *IniSection;
    EConfigType Type;
  };

  class Configuration_ {
    DEFINE_TAG("CFG");
    friend class AutoGuard<Configuration_>;
    public:
      using Guard = AutoGuard<Configuration_>;
      static Configuration_ &Instance();
      bool Init();
      template<typename T> T Get();
      bool CreateConfig();
      bool FactoryReset();
      const char *GetConfig(const char *tKey);
      bool SetConfig(const char *tKey, const char *tValue);
      bool SaveImageName(const char *tValue);
      bool SaveSession(uint32_t tValue);
      uint32_t GetSession();
      const char *PrepareAllConfigToINI();
      SAppConfig LoadConfigFromINI(const char *tFileName = nullptr);
      bool SaveAllConfig(const SAppConfig &tConfig);
    private:
      Configuration_();
      Configuration_(const Configuration_&) = delete;
      Configuration_ &operator=(const Configuration_&) = delete;
      ~Configuration_();
      Preferences mConfig;
      const char *mLabel = "cfg";
      const char *mPartLabel = "nvs";
      mutable SemaphoreHandle_t mMutex;
      static void Lock();
      static void Unlock();
      static constexpr const char *kNvsDeviceConfig = "dvc.cfg";
      static constexpr const char *kNvsDeviceAppName = "dvc.appname";
      static constexpr const char *kNvsDeviceVersion = "dvc.version";
      static constexpr const char *kNvsDisplayBrightness = "dsp.jpg.brght";
      static constexpr const char *kNvsDisplayContrast = "dsp.jpg.cntrst";
      static constexpr const char *kNvsDisplayGamma = "dsp.jpg.gmm";
      static constexpr const char *kNvsDisplayFile = "dsp.file";
      static constexpr const char *kNvsTimeServer = "tme.server";
      static constexpr const char *kNvsTimePort = "tme.port";
      static constexpr const char *kNvsTimeGmtOffset = "tme.gmt.offset";
      static constexpr const char *kNvsTimeUpdate = "tme.update";
      static constexpr const char *kNvsConApEnable = "con.ap.en";
      static constexpr const char *kNvsConApSsid = "con.ap.ssid";
      static constexpr const char *kNvsConApPass = "con.ap.pass";
      static constexpr const char *kNvsConApIp = "con.ap.ip";
      static constexpr const char *kNvsConApGw = "con.ap.gw";
      static constexpr const char *kNvsConApSubnet = "con.ap.subnet";
      static constexpr const char *kNvsConStaSsid = "con.sta.ssid";
      static constexpr const char *kNvsConStaPass = "con.sta.pass";
      static constexpr const char *kNvsConStaEnable = "con.sta.en";
      static constexpr const char *kNvsConStaIp = "con.sta.ip";
      static constexpr const char *kNvsConStaGw = "con.sta.gw";
      static constexpr const char *kNvsConStaSubnet = "con.sta.subnet";
      static constexpr const char *kNvsConStaDns1 = "con.sta.dns1";
      static constexpr const char *kNvsConStaDns2 = "con.sta.dns2";
      static constexpr const char *kNvsConMdnsEnable = "con.mdns.en";
      static constexpr const char *kNvsConMdnsName = "con.mdns.name";
      static constexpr const char *kNvsTimerWake = "tmr.wake";
      static constexpr const char *kNvsTimerWakeHour = "tmr.wake.hr";
      static constexpr const char *kNvsTelnetEnable = "tln.en";
      static constexpr const char *kNvsTelnetPort = "tln.port";
      static constexpr const char *kNvsTelnetUsername = "tln.username";
      static constexpr const char *kNvsTelnetPassword = "tln.password";
      static constexpr const char *kNvsTelnetSession = "tln.session";
      static constexpr const char *kNvsTelnetAuthSession = "tln.auth.sssn";
      static constexpr const char *kNvsFtpEnable = "ftp.en";
      static constexpr const char *kNvsFtpPort = "ftp.port";
      static constexpr const char *kNvsFtpUsername = "ftp.username";
      static constexpr const char *kNvsFtpPassword = "ftp.password";
      static const std::vector<SConfigKeyMappingEntry> &GetKeyMapping();
      static SAppConfig GetDefaultConfig();
      static void ApplyINIValue(SAppConfig &tConfig, const char *tSection, const SConfigKeyMappingEntry &tEntry, const char *tValue);
      static bool ReadINIFile(const char *tFileName, SAppConfig &tConfig);
      bool Begin(bool tReadOnly);
      void End();
      void AccessConfig(bool tReadOnly, std::function<void()> tAction);
      static bool ParseLine(char *tLine, char *tSection, char *tKey, char *tValue);
      static void TrimValue(char *tValue);
  };

  template<> SAppConfig Configuration_::Get<SAppConfig>();
  template<> SDeviceConfig Configuration_::Get<SDeviceConfig>();
  template<> SConnectionConfig Configuration_::Get<SConnectionConfig>();
  template<> SNTPConfig Configuration_::Get<SNTPConfig>();
  template<> SDisplayConfig Configuration_::Get<SDisplayConfig>();
  template<> STimerConfig Configuration_::Get<STimerConfig>();
  template<> SStorageConfig Configuration_::Get<SStorageConfig>();

};

#endif
