#include <unity.h>
#include <cstdint>

enum class EMaintenanceAction : uint8_t {
  None = 0,
  ExitToPhotoFrame,
  RebootForInactivity
};

static bool HasElapsedMs(uint32_t tStart, uint32_t tNow, uint32_t tTimeout) {
  if (tNow >= tStart) return (tNow - tStart) >= tTimeout;
  return ((UINT32_MAX - tStart) + tNow + 1U) >= tTimeout;
}

static EMaintenanceAction EvaluateMaintenanceAction(bool tExitRequested, uint32_t tLastActivityMs, uint32_t tNowMs, uint32_t tTimeoutMs) {
  if (tExitRequested) return EMaintenanceAction::ExitToPhotoFrame;
  if (HasElapsedMs(tLastActivityMs, tNowMs, tTimeoutMs)) return EMaintenanceAction::RebootForInactivity;
  return EMaintenanceAction::None;
}

void test_MaintenanceAction_none_when_no_exit_and_not_elapsed() {
  const auto tAction = EvaluateMaintenanceAction(false, 1000U, 1200U, 1000U);
  TEST_ASSERT_EQUAL(EMaintenanceAction::None, tAction);
}

void test_MaintenanceAction_exit_priority_over_inactivity() {
  const auto tAction = EvaluateMaintenanceAction(true, 1000U, 5000U, 1000U);
  TEST_ASSERT_EQUAL(EMaintenanceAction::ExitToPhotoFrame, tAction);
}

void test_MaintenanceAction_reboot_when_inactivity_elapsed() {
  const auto tAction = EvaluateMaintenanceAction(false, 1000U, 2500U, 1000U);
  TEST_ASSERT_EQUAL(EMaintenanceAction::RebootForInactivity, tAction);
}

void test_MaintenanceAction_reboot_on_exact_boundary() {
  const auto tAction = EvaluateMaintenanceAction(false, 1000U, 2000U, 1000U);
  TEST_ASSERT_EQUAL(EMaintenanceAction::RebootForInactivity, tAction);
}

void test_MaintenanceAction_wraparound_elapsed() {
  const uint32_t tStart = 0xFFFFFF00U;
  const uint32_t tNow = 0x00000100U;
  const auto tAction = EvaluateMaintenanceAction(false, tStart, tNow, 400U);
  TEST_ASSERT_EQUAL(EMaintenanceAction::RebootForInactivity, tAction);
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_MaintenanceAction_none_when_no_exit_and_not_elapsed);
  RUN_TEST(test_MaintenanceAction_exit_priority_over_inactivity);
  RUN_TEST(test_MaintenanceAction_reboot_when_inactivity_elapsed);
  RUN_TEST(test_MaintenanceAction_reboot_on_exact_boundary);
  RUN_TEST(test_MaintenanceAction_wraparound_elapsed);
  return UNITY_END();
}
