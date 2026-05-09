#include <unity.h>
#include <cstring>
#include <cstdint>
#include <cctype>

bool IfHaveArguments(const char *tInput) {
  for (size_t i = 0; i < strlen(tInput); i++) {
    if (isspace((int)tInput[i])) return true;
  }
  return false;
}
void ParseCommandName(const char *tInput, char *tCmdName, size_t tCmdNameSize) {
  if (!tInput || !tCmdName || tCmdNameSize == 0) return;
  strncpy(tCmdName, tInput, tCmdNameSize - 1);
  tCmdName[tCmdNameSize - 1] = '\0';
  char *tSpace = strchr(tCmdName, ' ');
  if (tSpace) *tSpace = '\0';
}
const char *GetArguments(const char *tInput) {
  if (!tInput) return "";
  const char *tSpace = strchr(tInput, ' ');
  if (tSpace) {
    while (*tSpace == ' ') tSpace++;
    return tSpace;
  }
  return "";
}

void test_IfHaveArguments_no_args() {
  TEST_ASSERT_FALSE(IfHaveArguments("help"));
  TEST_ASSERT_FALSE(IfHaveArguments("list"));
  TEST_ASSERT_FALSE(IfHaveArguments("exit"));
}

void test_IfHaveArguments_with_space() {
  TEST_ASSERT_TRUE(IfHaveArguments("cat file.txt"));
  TEST_ASSERT_TRUE(IfHaveArguments("config set key=value"));
  TEST_ASSERT_TRUE(IfHaveArguments("ls /images"));
}

void test_IfHaveArguments_only_space() {
  TEST_ASSERT_TRUE(IfHaveArguments(" "));
  TEST_ASSERT_TRUE(IfHaveArguments("  "));
}

void test_IfHaveArguments_tab() {
  TEST_ASSERT_TRUE(IfHaveArguments("cat\tfile.txt"));
  TEST_ASSERT_TRUE(IfHaveArguments("\t"));
}

void test_IfHaveArguments_empty() {
  TEST_ASSERT_FALSE(IfHaveArguments(""));
}

void test_IfHaveArguments_trailing_space() {
  TEST_ASSERT_TRUE(IfHaveArguments("help "));
}

void test_ParseCommandName_simple() {
  char cmd[32];
  ParseCommandName("help", cmd, sizeof(cmd));
  TEST_ASSERT_EQUAL_STRING("help", cmd);
}

void test_ParseCommandName_with_args() {
  char cmd[32];
  ParseCommandName("cat file.txt", cmd, sizeof(cmd));
  TEST_ASSERT_EQUAL_STRING("cat", cmd);
}

void test_ParseCommandName_multiple_args() {
  char cmd[32];
  ParseCommandName("config set key value", cmd, sizeof(cmd));
  TEST_ASSERT_EQUAL_STRING("config", cmd);
}

void test_ParseCommandName_empty() {
  char cmd[32] = "initial";
  ParseCommandName("", cmd, sizeof(cmd));
  TEST_ASSERT_EQUAL_STRING("", cmd);
}

void test_ParseCommandName_null() {
  char cmd[32] = "initial";
  ParseCommandName(nullptr, cmd, sizeof(cmd));
  TEST_ASSERT_EQUAL_STRING("initial", cmd);
}

void test_ParseCommandName_small_buffer() {
  char cmd[4];
  ParseCommandName("longcommand args", cmd, sizeof(cmd));
  TEST_ASSERT_EQUAL_STRING("lon", cmd);
}

void test_GetArguments_no_args() {
  const char *args = GetArguments("help");
  TEST_ASSERT_EQUAL_STRING("", args);
}

void test_GetArguments_single_arg() {
  const char *args = GetArguments("cat file.txt");
  TEST_ASSERT_EQUAL_STRING("file.txt", args);
}

void test_GetArguments_multiple_args() {
  const char *args = GetArguments("config set value");
  TEST_ASSERT_EQUAL_STRING("set value", args);
}

void test_GetArguments_extra_spaces() {
  const char *args = GetArguments("cat   file.txt");
  TEST_ASSERT_EQUAL_STRING("file.txt", args);
}

void test_GetArguments_null() {
  const char *args = GetArguments(nullptr);
  TEST_ASSERT_EQUAL_STRING("", args);
}

void test_GetArguments_empty() {
  const char *args = GetArguments("");
  TEST_ASSERT_EQUAL_STRING("", args);
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_IfHaveArguments_no_args);
  RUN_TEST(test_IfHaveArguments_with_space);
  RUN_TEST(test_IfHaveArguments_only_space);
  RUN_TEST(test_IfHaveArguments_tab);
  RUN_TEST(test_IfHaveArguments_empty);
  RUN_TEST(test_IfHaveArguments_trailing_space);
  RUN_TEST(test_ParseCommandName_simple);
  RUN_TEST(test_ParseCommandName_with_args);
  RUN_TEST(test_ParseCommandName_multiple_args);
  RUN_TEST(test_ParseCommandName_empty);
  RUN_TEST(test_ParseCommandName_null);
  RUN_TEST(test_ParseCommandName_small_buffer);
  RUN_TEST(test_GetArguments_no_args);
  RUN_TEST(test_GetArguments_single_arg);
  RUN_TEST(test_GetArguments_multiple_args);
  RUN_TEST(test_GetArguments_extra_spaces);
  RUN_TEST(test_GetArguments_null);
  RUN_TEST(test_GetArguments_empty);

  return UNITY_END();
}

