#include <unity.h>
#include <cstdint>
#include <cstddef>

struct SToneStep {
  constexpr SToneStep(uint16_t tFrequencyHz = 0, uint16_t tDurationMs = 0, uint16_t tPauseMs = 0, uint8_t tDutyPct = 50)
    : FrequencyHz(tFrequencyHz), DurationMs(tDurationMs), PauseMs(tPauseMs), DutyPct(tDutyPct) {}
  uint16_t FrequencyHz;
  uint16_t DurationMs;
  uint16_t PauseMs;
  uint8_t DutyPct;
};

static uint8_t ClampDutyPct(uint8_t tDutyPct) {
  return (tDutyPct > 100U) ? 100U : tDutyPct;
}

static uint32_t DutyToLdcValue(uint8_t tDutyPct, uint8_t tResolutionBits) {
  const uint32_t tMaxDuty = (1U << tResolutionBits) - 1U;
  return (tMaxDuty * ClampDutyPct(tDutyPct)) / 100U;
}

static bool IsToneStepAudible(const SToneStep &tStep) {
  return tStep.FrequencyHz > 0U && tStep.DurationMs > 0U;
}

void test_ToneStep_defaults() {
  SToneStep tStep;
  TEST_ASSERT_EQUAL_UINT16(0, tStep.FrequencyHz);
  TEST_ASSERT_EQUAL_UINT16(0, tStep.DurationMs);
  TEST_ASSERT_EQUAL_UINT16(0, tStep.PauseMs);
  TEST_ASSERT_EQUAL_UINT8(50, tStep.DutyPct);
}

void test_ClampDutyPct_over_100() {
  TEST_ASSERT_EQUAL_UINT8(100, ClampDutyPct(140));
}

void test_ClampDutyPct_keeps_value_in_range() {
  TEST_ASSERT_EQUAL_UINT8(75, ClampDutyPct(75));
}

void test_DutyToLdcValue_10bit_50pct() {
  const uint32_t tDuty = DutyToLdcValue(50, 10);
  TEST_ASSERT_EQUAL_UINT32(511, tDuty);
}

void test_DutyToLdcValue_10bit_100pct() {
  const uint32_t tDuty = DutyToLdcValue(100, 10);
  TEST_ASSERT_EQUAL_UINT32(1023, tDuty);
}

void test_IsToneStepAudible_rejects_zero_frequency() {
  const SToneStep tStep(0, 80, 0, 50);
  TEST_ASSERT_FALSE(IsToneStepAudible(tStep));
}

void test_IsToneStepAudible_rejects_zero_duration() {
  const SToneStep tStep(1800, 0, 0, 50);
  TEST_ASSERT_FALSE(IsToneStepAudible(tStep));
}

void test_IsToneStepAudible_accepts_valid_step() {
  const SToneStep tStep(1800, 80, 0, 50);
  TEST_ASSERT_TRUE(IsToneStepAudible(tStep));
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_ToneStep_defaults);
  RUN_TEST(test_ClampDutyPct_over_100);
  RUN_TEST(test_ClampDutyPct_keeps_value_in_range);
  RUN_TEST(test_DutyToLdcValue_10bit_50pct);
  RUN_TEST(test_DutyToLdcValue_10bit_100pct);
  RUN_TEST(test_IsToneStepAudible_rejects_zero_frequency);
  RUN_TEST(test_IsToneStepAudible_rejects_zero_duration);
  RUN_TEST(test_IsToneStepAudible_accepts_valid_step);
  return UNITY_END();
}
