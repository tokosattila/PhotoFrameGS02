#include <unity.h>
#include <cstdint>
#include <functional>

struct SInitRetryResult {
  bool Success;
  uint8_t AttemptsUsed;
};

static SInitRetryResult RunInitRetry(uint8_t tMaxAttempts, std::function<bool(uint8_t)> tInit) {
  SInitRetryResult tResult{false, 0};
  for (uint8_t tAttempt = 0; tAttempt < tMaxAttempts; ++tAttempt) {
    tResult.AttemptsUsed = static_cast<uint8_t>(tAttempt + 1);
    if (tInit(tAttempt)) {
      tResult.Success = true;
      return tResult;
    }
  }
  return tResult;
}

void test_InitRetry_SucceedsOnFirstAttempt() {
  const auto tResult = RunInitRetry(3, [](uint8_t) { return true; });
  TEST_ASSERT_TRUE(tResult.Success);
  TEST_ASSERT_EQUAL_UINT8(1, tResult.AttemptsUsed);
}

void test_InitRetry_SucceedsOnSecondAttempt() {
  const auto tResult = RunInitRetry(3, [](uint8_t tAttempt) { return tAttempt == 1; });
  TEST_ASSERT_TRUE(tResult.Success);
  TEST_ASSERT_EQUAL_UINT8(2, tResult.AttemptsUsed);
}

void test_InitRetry_SucceedsOnLastAttempt() {
  const auto tResult = RunInitRetry(3, [](uint8_t tAttempt) { return tAttempt == 2; });
  TEST_ASSERT_TRUE(tResult.Success);
  TEST_ASSERT_EQUAL_UINT8(3, tResult.AttemptsUsed);
}

void test_InitRetry_FailsAfterAllAttempts() {
  const auto tResult = RunInitRetry(3, [](uint8_t) { return false; });
  TEST_ASSERT_FALSE(tResult.Success);
  TEST_ASSERT_EQUAL_UINT8(3, tResult.AttemptsUsed);
}

void test_InitRetry_DoesNotRetryWhenMaxAttemptsIsZero() {
  bool tCalled = false;
  const auto tResult = RunInitRetry(0, [&](uint8_t) { tCalled = true; return true; });
  TEST_ASSERT_FALSE(tResult.Success);
  TEST_ASSERT_EQUAL_UINT8(0, tResult.AttemptsUsed);
  TEST_ASSERT_FALSE(tCalled);
}

void test_InitRetry_StopsCallingInitAfterSuccess() {
  uint8_t tCallCount = 0;
  const auto tResult = RunInitRetry(3, [&](uint8_t) { tCallCount++; return true; });
  TEST_ASSERT_TRUE(tResult.Success);
  TEST_ASSERT_EQUAL_UINT8(1, tCallCount);
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_InitRetry_SucceedsOnFirstAttempt);
  RUN_TEST(test_InitRetry_SucceedsOnSecondAttempt);
  RUN_TEST(test_InitRetry_SucceedsOnLastAttempt);
  RUN_TEST(test_InitRetry_FailsAfterAllAttempts);
  RUN_TEST(test_InitRetry_DoesNotRetryWhenMaxAttemptsIsZero);
  RUN_TEST(test_InitRetry_StopsCallingInitAfterSuccess);
  return UNITY_END();
}
