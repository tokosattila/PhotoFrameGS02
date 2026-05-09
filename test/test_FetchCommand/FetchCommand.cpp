#include <unity.h>
#include <cstring>
#include <cstdio>
#include <cstdint>

#include "../mocks/MockString.h"
#include "../mocks/MockWiFiClient.h"

#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_WHITE "\033[37m"

namespace FetchCommandTest {

  const char *SkipWhitespace(const char *tPtr) {
    while (*tPtr == ' ' || *tPtr == '\t') ++tPtr;
    return tPtr;
  }

  const char *SkipNonWhitespace(const char *tPtr) {
    while (*tPtr && *tPtr != ' ' && *tPtr != '\t') ++tPtr;
    return tPtr;
  }

  void PrintError(MockWiFiClient &tClient, const char *tMessage) {
    tClient.printf(COLOR_RED "\r\n  Error: %s\r\n" COLOR_YELLOW "  Usage: fetch <url> [filename]\r\n\r\n" COLOR_WHITE, tMessage);
  }

  static constexpr uint16_t kHttpPort = 80;
  static constexpr uint16_t kHttpsPort = 443;

  bool ParseUrl(const char *tUrl, String &tHost, String &tPath, uint16_t &tPort, bool &tIsHttps, MockWiFiClient &tClient) {
    const char *tHostStart;
    if (strncmp(tUrl, "https://", 8) == 0) {
      tHostStart = tUrl + 8;
      tPort = kHttpsPort;
      tIsHttps = true;
    } else if (strncmp(tUrl, "http://", 7) == 0) {
      tHostStart = tUrl + 7;
      tPort = kHttpPort;
      tIsHttps = false;
    } else {
      PrintError(tClient, "URL must start with -> http:// or https://");
      return false;
    }
    const char *tPathStart = strchr(tHostStart, '/');
    if (tPathStart) {
      tHost = String(tHostStart, tPathStart - tHostStart);
      tPath = String(tPathStart);
    } else {
      tHost = String(tHostStart);
      tPath = "/";
    }
    return true;
  }

  bool ParseArguments(const char *tArguments, char *tUrl, size_t tUrlSize, char *tFilename, size_t tFilenameSize, MockWiFiClient &tClient) {
    if (!tArguments || !*tArguments) {
      PrintError(tClient, "Missing URL");
      return false;
    }
    const char *tPtr = SkipWhitespace(tArguments);
    tPtr = SkipNonWhitespace(tPtr);
    tPtr = SkipWhitespace(tPtr);
    const char *tUrlStart = tPtr;
    tPtr = SkipNonWhitespace(tPtr);
    size_t tUrlLen = tPtr - tUrlStart;
    if (tUrlLen == 0 || tUrlLen >= tUrlSize) {
      PrintError(tClient, "Invalid or too long URL");
      return false;
    }
    memcpy(tUrl, tUrlStart, tUrlLen);
    tUrl[tUrlLen] = '\0';
    tPtr = SkipWhitespace(tPtr);
    if (*tPtr) {
      const char *tFilenameStart = tPtr;
      tPtr = SkipNonWhitespace(tPtr);
      size_t tFilenameLen = tPtr - tFilenameStart;
      if (tFilenameLen >= tFilenameSize) {
        PrintError(tClient, "Filename too long");
        return false;
      }
      if (*SkipWhitespace(tPtr)) {
        PrintError(tClient, "Too many arguments");
        return false;
      }
      memcpy(tFilename, tFilenameStart, tFilenameLen);
      tFilename[tFilenameLen] = '\0';
    } else tFilename[0] = '\0';
    return true;
  }

}

void test_SkipWhitespace_no_whitespace() {
  const char *input = "hello";
  const char *result = FetchCommandTest::SkipWhitespace(input);
  TEST_ASSERT_EQUAL_PTR(input, result);
}

void test_SkipWhitespace_leading_spaces() {
  const char *input = "   hello";
  const char *result = FetchCommandTest::SkipWhitespace(input);
  TEST_ASSERT_EQUAL_STRING("hello", result);
}

void test_SkipWhitespace_leading_tabs() {
  const char *input = "\t\thello";
  const char *result = FetchCommandTest::SkipWhitespace(input);
  TEST_ASSERT_EQUAL_STRING("hello", result);
}

void test_SkipWhitespace_mixed() {
  const char *input = " \t \t hello";
  const char *result = FetchCommandTest::SkipWhitespace(input);
  TEST_ASSERT_EQUAL_STRING("hello", result);
}

void test_SkipWhitespace_only_whitespace() {
  const char *input = "   \t  ";
  const char *result = FetchCommandTest::SkipWhitespace(input);
  TEST_ASSERT_EQUAL_STRING("", result);
}

void test_SkipWhitespace_empty() {
  const char *input = "";
  const char *result = FetchCommandTest::SkipWhitespace(input);
  TEST_ASSERT_EQUAL_STRING("", result);
}

void test_SkipNonWhitespace_simple() {
  const char *input = "hello world";
  const char *result = FetchCommandTest::SkipNonWhitespace(input);
  TEST_ASSERT_EQUAL_STRING(" world", result);
}

void test_SkipNonWhitespace_no_whitespace() {
  const char *input = "hello";
  const char *result = FetchCommandTest::SkipNonWhitespace(input);
  TEST_ASSERT_EQUAL_STRING("", result);
}

void test_SkipNonWhitespace_starts_with_space() {
  const char *input = " hello";
  const char *result = FetchCommandTest::SkipNonWhitespace(input);
  TEST_ASSERT_EQUAL_PTR(input, result);
}

void test_SkipNonWhitespace_tab_separator() {
  const char *input = "hello\tworld";
  const char *result = FetchCommandTest::SkipNonWhitespace(input);
  TEST_ASSERT_EQUAL_STRING("\tworld", result);
}

void test_SkipNonWhitespace_empty() {
  const char *input = "";
  const char *result = FetchCommandTest::SkipNonWhitespace(input);
  TEST_ASSERT_EQUAL_STRING("", result);
}

void test_ParseUrl_http_simple() {
  MockWiFiClient client;
  String host, path;
  uint16_t port;
  bool isHttps;
  bool result = FetchCommandTest::ParseUrl("http://example.com", host, path, port, isHttps, client);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("example.com", host.c_str());
  TEST_ASSERT_EQUAL_STRING("/", path.c_str());
  TEST_ASSERT_EQUAL_UINT16(80, port);
  TEST_ASSERT_FALSE(isHttps);
}

void test_ParseUrl_http_with_path() {
  MockWiFiClient client;
  String host, path;
  uint16_t port;
  bool isHttps;
  bool result = FetchCommandTest::ParseUrl("http://example.com/images/photo.jpg", host, path, port, isHttps, client);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("example.com", host.c_str());
  TEST_ASSERT_EQUAL_STRING("/images/photo.jpg", path.c_str());
  TEST_ASSERT_EQUAL_UINT16(80, port);
  TEST_ASSERT_FALSE(isHttps);
}

void test_ParseUrl_https_simple() {
  MockWiFiClient client;
  String host, path;
  uint16_t port;
  bool isHttps;
  bool result = FetchCommandTest::ParseUrl("https://secure.example.com", host, path, port, isHttps, client);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("secure.example.com", host.c_str());
  TEST_ASSERT_EQUAL_STRING("/", path.c_str());
  TEST_ASSERT_EQUAL_UINT16(443, port);
  TEST_ASSERT_TRUE(isHttps);
}

void test_ParseUrl_https_with_path() {
  MockWiFiClient client;
  String host, path;
  uint16_t port;
  bool isHttps;
  bool result = FetchCommandTest::ParseUrl("https://api.example.com/v1/data", host, path, port, isHttps, client);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("api.example.com", host.c_str());
  TEST_ASSERT_EQUAL_STRING("/v1/data", path.c_str());
  TEST_ASSERT_EQUAL_UINT16(443, port);
  TEST_ASSERT_TRUE(isHttps);
}

void test_ParseUrl_invalid_protocol() {
  MockWiFiClient client;
  String host, path;
  uint16_t port;
  bool isHttps;
  bool result = FetchCommandTest::ParseUrl("ftp://example.com", host, path, port, isHttps, client);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_TRUE(client.contains("Error"));
  TEST_ASSERT_TRUE(client.contains("http://"));
}

void test_ParseUrl_no_protocol() {
  MockWiFiClient client;
  String host, path;
  uint16_t port;
  bool isHttps;
  bool result = FetchCommandTest::ParseUrl("example.com/path", host, path, port, isHttps, client);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_TRUE(client.contains("Error"));
}

void test_ParseUrl_complex_path() {
  MockWiFiClient client;
  String host, path;
  uint16_t port;
  bool isHttps;
  bool result = FetchCommandTest::ParseUrl("http://cdn.site.org/assets/img/2024/photo_01.jpg?v=2", host, path, port, isHttps, client);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("cdn.site.org", host.c_str());
  TEST_ASSERT_EQUAL_STRING("/assets/img/2024/photo_01.jpg?v=2", path.c_str());
}

void test_ParseArguments_url_only() {
  MockWiFiClient client;
  char url[256], filename[128];
  bool result = FetchCommandTest::ParseArguments("fetch http://example.com/img.jpg", url, sizeof(url), filename, sizeof(filename), client);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("http://example.com/img.jpg", url);
  TEST_ASSERT_EQUAL_STRING("", filename);
}

void test_ParseArguments_url_and_filename() {
  MockWiFiClient client;
  char url[256], filename[128];
  bool result = FetchCommandTest::ParseArguments("fetch http://example.com/img.jpg myfile.jpg", url, sizeof(url), filename, sizeof(filename), client);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("http://example.com/img.jpg", url);
  TEST_ASSERT_EQUAL_STRING("myfile.jpg", filename);
}

void test_ParseArguments_extra_spaces() {
  MockWiFiClient client;
  char url[256], filename[128];
  bool result = FetchCommandTest::ParseArguments("fetch   http://example.com/img.jpg   output.jpg", url, sizeof(url), filename, sizeof(filename), client);
  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL_STRING("http://example.com/img.jpg", url);
  TEST_ASSERT_EQUAL_STRING("output.jpg", filename);
}

void test_ParseArguments_missing_url() {
  MockWiFiClient client;
  char url[256], filename[128];
  bool result = FetchCommandTest::ParseArguments("", url, sizeof(url), filename, sizeof(filename), client);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_TRUE(client.contains("Missing URL"));
}

void test_ParseArguments_null_input() {
  MockWiFiClient client;
  char url[256], filename[128];
  bool result = FetchCommandTest::ParseArguments(nullptr, url, sizeof(url), filename, sizeof(filename), client);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_TRUE(client.contains("Missing URL"));
}

void test_ParseArguments_too_many_args() {
  MockWiFiClient client;
  char url[256], filename[128];
  bool result = FetchCommandTest::ParseArguments("fetch http://example.com/img.jpg file.jpg extra", url, sizeof(url), filename, sizeof(filename), client);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_TRUE(client.contains("Too many arguments"));
}

void test_ParseArguments_url_too_long() {
  MockWiFiClient client;
  char url[32], filename[128];
  bool result = FetchCommandTest::ParseArguments("fetch http://very-long-domain-name-that-exceeds-buffer.com/path/to/file.jpg", url, sizeof(url), filename, sizeof(filename), client);
  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_TRUE(client.contains("Invalid or too long URL"));
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_SkipWhitespace_no_whitespace);
  RUN_TEST(test_SkipWhitespace_leading_spaces);
  RUN_TEST(test_SkipWhitespace_leading_tabs);
  RUN_TEST(test_SkipWhitespace_mixed);
  RUN_TEST(test_SkipWhitespace_only_whitespace);
  RUN_TEST(test_SkipWhitespace_empty);
  RUN_TEST(test_SkipNonWhitespace_simple);
  RUN_TEST(test_SkipNonWhitespace_no_whitespace);
  RUN_TEST(test_SkipNonWhitespace_starts_with_space);
  RUN_TEST(test_SkipNonWhitespace_tab_separator);
  RUN_TEST(test_SkipNonWhitespace_empty);
  RUN_TEST(test_ParseUrl_http_simple);
  RUN_TEST(test_ParseUrl_http_with_path);
  RUN_TEST(test_ParseUrl_https_simple);
  RUN_TEST(test_ParseUrl_https_with_path);
  RUN_TEST(test_ParseUrl_invalid_protocol);
  RUN_TEST(test_ParseUrl_no_protocol);
  RUN_TEST(test_ParseUrl_complex_path);
  RUN_TEST(test_ParseArguments_url_only);
  RUN_TEST(test_ParseArguments_url_and_filename);
  RUN_TEST(test_ParseArguments_extra_spaces);
  RUN_TEST(test_ParseArguments_missing_url);
  RUN_TEST(test_ParseArguments_null_input);
  RUN_TEST(test_ParseArguments_too_many_args);
  RUN_TEST(test_ParseArguments_url_too_long);
  return UNITY_END();
}
