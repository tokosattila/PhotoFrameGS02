#include <unity.h>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <cstdio>

namespace App {
  enum class ELogLevel : uint8_t {
    Boot = 0,
    Halt,
    Storage,
    Wifi,
    Ntp,
    Rtc,
    Battery,
    Image,
    Sleep,
    Ota,
    Warn,
    Error
  };
}

bool BuildLogFilePath(char *tPath, size_t tSize, uint8_t tRollIndex = 0) {
  time_t tNow = time(nullptr);
  struct tm tTm = {};
  if (tNow >= 1000000000L) {
    #ifdef _WIN32
      localtime_s(&tTm, &tNow);
    #else
      localtime_r(&tNow, &tTm);
    #endif
  } else memset(&tTm, 0, sizeof(tTm));
  if (tRollIndex == 0) snprintf(tPath, tSize, "logs/%04d/%02d/%02d/%04d%02d%02d.log", tTm.tm_year + 1900, tTm.tm_mon + 1, tTm.tm_mday, tTm.tm_year + 1900, tTm.tm_mon + 1, tTm.tm_mday);
  else snprintf(tPath, tSize, "logs/%04d/%02d/%02d/%04d%02d%02d_%u.log", tTm.tm_year + 1900, tTm.tm_mon + 1, tTm.tm_mday, tTm.tm_year + 1900, tTm.tm_mon + 1, tTm.tm_mday, (unsigned)tRollIndex);
  return true;
}

void setUp(void) {
}

void tearDown(void) {
}

void test_LogFilePath_GenerateBaseFile(void) {
  char tPath[96] = "";
  TEST_ASSERT_TRUE(BuildLogFilePath(tPath, sizeof(tPath), 0));
  TEST_ASSERT_NOT_NULL(strstr(tPath, "logs/"));
  TEST_ASSERT_NOT_NULL(strstr(tPath, ".log"));
  TEST_ASSERT_NULL(strstr(tPath, "_0.log"));
}

void test_LogFilePath_GenerateRolloverFile(void) {
  char tPath[96] = "";
  TEST_ASSERT_TRUE(BuildLogFilePath(tPath, sizeof(tPath), 1));
  TEST_ASSERT_NOT_NULL(strstr(tPath, "logs/"));
  TEST_ASSERT_NOT_NULL(strstr(tPath, "_1.log"));
}

void test_LogFilePath_BufferSize(void) {
  char tPath[96] = "";
  TEST_ASSERT_TRUE(BuildLogFilePath(tPath, sizeof(tPath), 5));
  TEST_ASSERT_NOT_NULL(tPath);
  TEST_ASSERT_GREATER_THAN(0, strlen(tPath));
}

void test_LogFilePath_MultipleRolloverIndices(void) {
  char tPath1[96] = "";
  char tPath2[96] = "";
  char tPath3[96] = "";
  BuildLogFilePath(tPath1, sizeof(tPath1), 0);
  BuildLogFilePath(tPath2, sizeof(tPath2), 1);
  BuildLogFilePath(tPath3, sizeof(tPath3), 2);
  TEST_ASSERT_NOT_EQUAL(0, strcmp(tPath1, tPath2));
  TEST_ASSERT_NOT_EQUAL(0, strcmp(tPath2, tPath3));
}

void test_LogLevel_EnumValues(void) {
  TEST_ASSERT_EQUAL(0, static_cast<uint8_t>(App::ELogLevel::Boot));
  TEST_ASSERT_EQUAL(1, static_cast<uint8_t>(App::ELogLevel::Halt));
  TEST_ASSERT_EQUAL(2, static_cast<uint8_t>(App::ELogLevel::Storage));
  TEST_ASSERT_EQUAL(9, static_cast<uint8_t>(App::ELogLevel::Ota));
  TEST_ASSERT_EQUAL(11, static_cast<uint8_t>(App::ELogLevel::Error));
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_LogFilePath_GenerateBaseFile);
  RUN_TEST(test_LogFilePath_GenerateRolloverFile);
  RUN_TEST(test_LogFilePath_BufferSize);
  RUN_TEST(test_LogFilePath_MultipleRolloverIndices);
  RUN_TEST(test_LogLevel_EnumValues);
  return UNITY_END();
}
