#include <unity.h>
#include <cstdint>

static uint32_t ReconcileBootCount(uint32_t tInMemory, uint32_t tPersisted) {
  if (tPersisted + 1U > tInMemory) tInMemory = tPersisted + 1U;
  return tInMemory;
}

void test_BootCountReconcile_FirstBoot_BothZero() {
  const uint32_t tInMemory = 1U;
  const uint32_t tPersisted = 0U;
  TEST_ASSERT_EQUAL_UINT32(1U, ReconcileBootCount(tInMemory, tPersisted));
}

void test_BootCountReconcile_PowerCycleResetsRtcRam() {
  const uint32_t tInMemory = 1U;
  const uint32_t tPersisted = 42U;
  TEST_ASSERT_EQUAL_UINT32(43U, ReconcileBootCount(tInMemory, tPersisted));
}

void test_BootCountReconcile_DeepSleepKeepsRtcRamAhead() {
  const uint32_t tInMemory = 100U;
  const uint32_t tPersisted = 50U;
  TEST_ASSERT_EQUAL_UINT32(100U, ReconcileBootCount(tInMemory, tPersisted));
}

void test_BootCountReconcile_EqualPersistedAndInMemory() {
  const uint32_t tInMemory = 7U;
  const uint32_t tPersisted = 6U;
  TEST_ASSERT_EQUAL_UINT32(7U, ReconcileBootCount(tInMemory, tPersisted));
}

void test_BootCountReconcile_PersistedOneAheadBumpsInMemory() {
  const uint32_t tInMemory = 5U;
  const uint32_t tPersisted = 5U;
  TEST_ASSERT_EQUAL_UINT32(6U, ReconcileBootCount(tInMemory, tPersisted));
}

void test_BootCountReconcile_Monotonic() {
  uint32_t tInMemory = 1U;
  uint32_t tPersisted = 0U;
  for (uint32_t i = 0; i < 10U; ++i) {
    tInMemory = ReconcileBootCount(tInMemory, tPersisted);
    tPersisted = tInMemory;
    tInMemory = 1U;
  }
  TEST_ASSERT_EQUAL_UINT32(10U, tPersisted);
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_BootCountReconcile_FirstBoot_BothZero);
  RUN_TEST(test_BootCountReconcile_PowerCycleResetsRtcRam);
  RUN_TEST(test_BootCountReconcile_DeepSleepKeepsRtcRamAhead);
  RUN_TEST(test_BootCountReconcile_EqualPersistedAndInMemory);
  RUN_TEST(test_BootCountReconcile_PersistedOneAheadBumpsInMemory);
  RUN_TEST(test_BootCountReconcile_Monotonic);
  return UNITY_END();
}
