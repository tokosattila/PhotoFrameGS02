#include <unity.h>
#include <cstring>
#include <cstdint>
#include <cstdio>

struct SRTCDateTime {
  uint16_t Year = 2026;
  uint8_t Month = 1;
  uint8_t Day = 1;
  uint8_t Hour = 0;
  uint8_t Minute = 0;
  uint8_t Second = 0;
  uint8_t DayOfWeek = 0;
};

enum class EDateParseResult {
  Success = 0,
  InvalidFormat,
  InvalidYear,
  InvalidMonth,
  InvalidDay,
  InvalidHour,
  InvalidMinute,
  InvalidSecond
};

EDateParseResult ParseDateTimeArgs(const char *tArgs, SRTCDateTime &tDateTime) {
  if (!tArgs || tArgs[0] == '\0') return EDateParseResult::InvalidFormat;
  int tYear, tMonth, tDay, tHour, tMin, tSec;
  if (sscanf(tArgs, "%d.%d.%d %d:%d:%d", &tYear, &tMonth, &tDay, &tHour, &tMin, &tSec) != 6) {
    return EDateParseResult::InvalidFormat;
  }
  if (tYear < 2026 || tYear > 2099) {
    return EDateParseResult::InvalidYear;
  }
  if (tMonth < 1 || tMonth > 12) {
    return EDateParseResult::InvalidMonth;
  }
  if (tDay < 1 || tDay > 31) {
    return EDateParseResult::InvalidDay;
  }
  if (tHour < 0 || tHour > 23) {
    return EDateParseResult::InvalidHour;
  }
  if (tMin < 0 || tMin > 59) {
    return EDateParseResult::InvalidMinute;
  }
  if (tSec < 0 || tSec > 59) {
    return EDateParseResult::InvalidSecond;
  }
  tDateTime.Year = tYear;
  tDateTime.Month = tMonth;
  tDateTime.Day = tDay;
  tDateTime.Hour = tHour;
  tDateTime.Minute = tMin;
  tDateTime.Second = tSec;
  return EDateParseResult::Success;
}

const char *GetDateSubcommand(const char *tArguments) {
  if (!tArguments) return "";
  const char *tPtr = tArguments;
  while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
  while (*tPtr != '\0' && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
  while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
  return tPtr;
}

bool IsSubcommand(const char *tPtr, const char *tSubcmd) {
  if (!tPtr || !tSubcmd) return false;
  return strcmp(tPtr, tSubcmd) == 0;
}

bool StartsWithSubcommand(const char *tPtr, const char *tSubcmd) {
  if (!tPtr || !tSubcmd) return false;
  return strncmp(tPtr, tSubcmd, strlen(tSubcmd)) == 0;
}

void test_ParseDateTimeArgs_valid_datetime() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.01.15 14:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::Success, result);
  TEST_ASSERT_EQUAL_UINT16(2026, dt.Year);
  TEST_ASSERT_EQUAL_UINT8(1, dt.Month);
  TEST_ASSERT_EQUAL_UINT8(15, dt.Day);
  TEST_ASSERT_EQUAL_UINT8(14, dt.Hour);
  TEST_ASSERT_EQUAL_UINT8(30, dt.Minute);
  TEST_ASSERT_EQUAL_UINT8(45, dt.Second);
}

void test_ParseDateTimeArgs_valid_max_values() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2099.12.31 23:59:59", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::Success, result);
  TEST_ASSERT_EQUAL_UINT16(2099, dt.Year);
  TEST_ASSERT_EQUAL_UINT8(12, dt.Month);
  TEST_ASSERT_EQUAL_UINT8(31, dt.Day);
  TEST_ASSERT_EQUAL_UINT8(23, dt.Hour);
  TEST_ASSERT_EQUAL_UINT8(59, dt.Minute);
  TEST_ASSERT_EQUAL_UINT8(59, dt.Second);
}

void test_ParseDateTimeArgs_valid_min_values() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.01.01 00:00:00", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::Success, result);
  TEST_ASSERT_EQUAL_UINT16(2026, dt.Year);
  TEST_ASSERT_EQUAL_UINT8(1, dt.Month);
  TEST_ASSERT_EQUAL_UINT8(1, dt.Day);
  TEST_ASSERT_EQUAL_UINT8(0, dt.Hour);
  TEST_ASSERT_EQUAL_UINT8(0, dt.Minute);
  TEST_ASSERT_EQUAL_UINT8(0, dt.Second);
}

void test_ParseDateTimeArgs_invalid_format_missing_time() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.01.15", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidFormat, result);
}

void test_ParseDateTimeArgs_invalid_format_wrong_separator() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026-01-15 14:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidFormat, result);
}

void test_ParseDateTimeArgs_invalid_format_empty() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidFormat, result);
}

void test_ParseDateTimeArgs_invalid_format_null() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs(nullptr, dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidFormat, result);
}

void test_ParseDateTimeArgs_year_too_low() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2025.01.15 14:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidYear, result);
}

void test_ParseDateTimeArgs_year_too_high() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2100.01.15 14:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidYear, result);
}

void test_ParseDateTimeArgs_month_zero() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.00.15 14:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidMonth, result);
}

void test_ParseDateTimeArgs_month_13() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.13.15 14:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidMonth, result);
}

void test_ParseDateTimeArgs_day_zero() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.01.00 14:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidDay, result);
}

void test_ParseDateTimeArgs_day_32() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.01.32 14:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidDay, result);
}

void test_ParseDateTimeArgs_hour_24() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.01.15 24:30:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidHour, result);
}

void test_ParseDateTimeArgs_minute_60() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.01.15 14:60:45", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidMinute, result);
}

void test_ParseDateTimeArgs_second_60() {
  SRTCDateTime dt;
  EDateParseResult result = ParseDateTimeArgs("2026.01.15 14:30:60", dt);
  TEST_ASSERT_EQUAL(EDateParseResult::InvalidSecond, result);
}

void test_GetDateSubcommand_rtc() {
  const char *result = GetDateSubcommand("date rtc");
  TEST_ASSERT_EQUAL_STRING("rtc", result);
}

void test_GetDateSubcommand_rtc_set() {
  const char *result = GetDateSubcommand("date rtc set 2026.01.15 14:30:45");
  TEST_ASSERT_TRUE(StartsWithSubcommand(result, "rtc set"));
}

void test_GetDateSubcommand_rtc_sync_from_ntp() {
  const char *result = GetDateSubcommand("date rtc sync-from-ntp");
  TEST_ASSERT_EQUAL_STRING("rtc sync-from-ntp", result);
}

void test_GetDateSubcommand_rtc_sync_to_system() {
  const char *result = GetDateSubcommand("date rtc sync-to-system");
  TEST_ASSERT_EQUAL_STRING("rtc sync-to-system", result);
}

void test_GetDateSubcommand_no_subcommand() {
  const char *result = GetDateSubcommand("date");
  TEST_ASSERT_EQUAL_STRING("", result);
}

void test_GetDateSubcommand_with_spaces() {
  const char *result = GetDateSubcommand("date   rtc");
  TEST_ASSERT_EQUAL_STRING("rtc", result);
}

void test_GetDateSubcommand_null() {
  const char *result = GetDateSubcommand(nullptr);
  TEST_ASSERT_EQUAL_STRING("", result);
}

void test_IsSubcommand_exact_match() {
  TEST_ASSERT_TRUE(IsSubcommand("rtc", "rtc"));
  TEST_ASSERT_TRUE(IsSubcommand("rtc sync-from-ntp", "rtc sync-from-ntp"));
}

void test_IsSubcommand_no_match() {
  TEST_ASSERT_FALSE(IsSubcommand("rtc", "ntp"));
  TEST_ASSERT_FALSE(IsSubcommand("rtc set", "rtc"));
}

void test_StartsWithSubcommand_match() {
  TEST_ASSERT_TRUE(StartsWithSubcommand("rtc set 2026.01.15", "rtc set "));
  TEST_ASSERT_TRUE(StartsWithSubcommand("rtc", "rtc"));
}

void test_StartsWithSubcommand_no_match() {
  TEST_ASSERT_FALSE(StartsWithSubcommand("ntp", "rtc"));
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_ParseDateTimeArgs_valid_datetime);
  RUN_TEST(test_ParseDateTimeArgs_valid_max_values);
  RUN_TEST(test_ParseDateTimeArgs_valid_min_values);
  RUN_TEST(test_ParseDateTimeArgs_invalid_format_missing_time);
  RUN_TEST(test_ParseDateTimeArgs_invalid_format_wrong_separator);
  RUN_TEST(test_ParseDateTimeArgs_invalid_format_empty);
  RUN_TEST(test_ParseDateTimeArgs_invalid_format_null);
  RUN_TEST(test_ParseDateTimeArgs_year_too_low);
  RUN_TEST(test_ParseDateTimeArgs_year_too_high);
  RUN_TEST(test_ParseDateTimeArgs_month_zero);
  RUN_TEST(test_ParseDateTimeArgs_month_13);
  RUN_TEST(test_ParseDateTimeArgs_day_zero);
  RUN_TEST(test_ParseDateTimeArgs_day_32);
  RUN_TEST(test_ParseDateTimeArgs_hour_24);
  RUN_TEST(test_ParseDateTimeArgs_minute_60);
  RUN_TEST(test_ParseDateTimeArgs_second_60);
  RUN_TEST(test_GetDateSubcommand_rtc);
  RUN_TEST(test_GetDateSubcommand_rtc_set);
  RUN_TEST(test_GetDateSubcommand_rtc_sync_from_ntp);
  RUN_TEST(test_GetDateSubcommand_rtc_sync_to_system);
  RUN_TEST(test_GetDateSubcommand_no_subcommand);
  RUN_TEST(test_GetDateSubcommand_with_spaces);
  RUN_TEST(test_GetDateSubcommand_null);
  RUN_TEST(test_IsSubcommand_exact_match);
  RUN_TEST(test_IsSubcommand_no_match);
  RUN_TEST(test_StartsWithSubcommand_match);
  RUN_TEST(test_StartsWithSubcommand_no_match);

  return UNITY_END();
}

