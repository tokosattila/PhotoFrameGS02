#include <unity.h>
#include <cstdint>
#include <cstring>

struct SToneConfig {
  bool Enable = true;
  SToneConfig() = default;
};

struct SAppConfig {
  SToneConfig Tone {};
  SAppConfig() = default;
};

class Tone_ {
  public:
    static Tone_ &Instance() {
      static Tone_ tInstance;
      return tInstance;
    }

    void SetEnabled(bool tEnabled) {
      mEnabled = tEnabled;
    }

    bool IsEnabled() const {
      return mEnabled;
    }

  private:
    bool mEnabled = true;
};

static Tone_ &TON = Tone_::Instance();

static SAppConfig mCfg {};

void ReloadConfig() {
  // Simulating CFG.Get<SAppConfig>()
  mCfg.Tone.Enable = true;
  TON.SetEnabled(mCfg.Tone.Enable);
}

void ReloadConfigWithDisabledTone() {
  mCfg.Tone.Enable = false;
  TON.SetEnabled(mCfg.Tone.Enable);
}

void test_ReloadConfig_tone_enable_default_true() {
  ReloadConfig();
  TEST_ASSERT_TRUE(TON.IsEnabled());
}

void test_ReloadConfig_tone_disabled_when_config_false() {
  ReloadConfigWithDisabledTone();
  TEST_ASSERT_FALSE(TON.IsEnabled());
}

void test_ReloadConfig_tone_enabled_when_config_true() {
  mCfg.Tone.Enable = true;
  ReloadConfig();
  TEST_ASSERT_TRUE(TON.IsEnabled());
}

void test_ReloadConfig_cfg_tone_structure_matches() {
  ReloadConfig();
  TEST_ASSERT_EQUAL(mCfg.Tone.Enable, TON.IsEnabled());
}

void test_ReloadConfig_tone_persists_after_multiple_reloads() {
  mCfg.Tone.Enable = true;
  ReloadConfig();
  TEST_ASSERT_TRUE(TON.IsEnabled());

  ReloadConfig();
  TEST_ASSERT_TRUE(TON.IsEnabled());

  ReloadConfigWithDisabledTone();
  TEST_ASSERT_FALSE(TON.IsEnabled());

  mCfg.Tone.Enable = true;
  ReloadConfig();
  TEST_ASSERT_TRUE(TON.IsEnabled());
}

void setUp(void) {
  // Reset tone to default
  TON.SetEnabled(true);
}

void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_ReloadConfig_tone_enable_default_true);
  RUN_TEST(test_ReloadConfig_tone_disabled_when_config_false);
  RUN_TEST(test_ReloadConfig_tone_enabled_when_config_true);
  RUN_TEST(test_ReloadConfig_cfg_tone_structure_matches);
  RUN_TEST(test_ReloadConfig_tone_persists_after_multiple_reloads);
  return UNITY_END();
}
