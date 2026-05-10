#include <unity.h>
#include <cstring>
#include <cstdint>
#include <vector>
#include <string>

enum class EFileSystemType : uint8_t {
  LittleFS = 1,
  SDCard = 2
};
struct MockStorageState {
  bool SDCardMounted = false;
  bool LittleFSMounted = false;
  bool SDCardHasImages = false;
  bool LittleFSHasImages = false;
  EFileSystemType DefaultFS = EFileSystemType::SDCard;
  bool FallbackEnabled = true;
};
struct StorageSelection {
  EFileSystemType ActiveType;
  bool Mounted;
  bool FallbackActive;
};

enum class ERenameBackend : uint8_t {
  None = 0,
  SDCard = 1,
  LittleFS = 2
};

StorageSelection SelectActiveStorage(const MockStorageState &state) {
  StorageSelection result = {EFileSystemType::LittleFS, false, false};

  if (state.DefaultFS == EFileSystemType::SDCard) {
    if (state.SDCardMounted) {
      if (state.SDCardHasImages) {
        result.ActiveType = EFileSystemType::SDCard;
        result.Mounted = true;
        result.FallbackActive = false;
        return result;
      }
      if (state.FallbackEnabled && state.LittleFSMounted && state.LittleFSHasImages) {
        result.ActiveType = EFileSystemType::LittleFS;
        result.Mounted = true;
        result.FallbackActive = true;
        return result;
      }
      result.ActiveType = EFileSystemType::SDCard;
      result.Mounted = true;
      result.FallbackActive = false;
      return result;
    }
    if (state.FallbackEnabled && state.LittleFSMounted) {
      result.ActiveType = EFileSystemType::LittleFS;
      result.Mounted = true;
      result.FallbackActive = true;
      return result;
    }
  } else {
    if (state.LittleFSMounted) {
      if (state.LittleFSHasImages) {
        result.ActiveType = EFileSystemType::LittleFS;
        result.Mounted = true;
        result.FallbackActive = false;
        return result;
      }
      if (state.FallbackEnabled && state.SDCardMounted && state.SDCardHasImages) {
        result.ActiveType = EFileSystemType::SDCard;
        result.Mounted = true;
        result.FallbackActive = true;
        return result;
      }
      result.ActiveType = EFileSystemType::LittleFS;
      result.Mounted = true;
      result.FallbackActive = false;
      return result;
    }
    if (state.FallbackEnabled && state.SDCardMounted) {
      result.ActiveType = EFileSystemType::SDCard;
      result.Mounted = true;
      result.FallbackActive = true;
      return result;
    }
  }

  return result;
}

ERenameBackend SelectRenameBackend(const StorageSelection &state) {
  if (!state.Mounted) return ERenameBackend::None;
  if (state.ActiveType == EFileSystemType::SDCard) return ERenameBackend::SDCard;
  return ERenameBackend::LittleFS;
}

void test_SDCard_primary_with_images() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::SDCard;
  state.SDCardMounted = true;
  state.SDCardHasImages = true;
  state.LittleFSMounted = true;
  state.LittleFSHasImages = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::SDCard, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_FALSE(result.FallbackActive);
}

void test_SDCard_primary_no_images_fallback_to_littlefs() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::SDCard;
  state.SDCardMounted = true;
  state.SDCardHasImages = false;
  state.LittleFSMounted = true;
  state.LittleFSHasImages = true;
  state.FallbackEnabled = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::LittleFS, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_TRUE(result.FallbackActive);
}

void test_SDCard_primary_not_mounted_fallback() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::SDCard;
  state.SDCardMounted = false;
  state.LittleFSMounted = true;
  state.LittleFSHasImages = true;
  state.FallbackEnabled = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::LittleFS, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_TRUE(result.FallbackActive);
}

void test_SDCard_primary_no_images_no_fallback_images() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::SDCard;
  state.SDCardMounted = true;
  state.SDCardHasImages = false;
  state.LittleFSMounted = true;
  state.LittleFSHasImages = false;
  state.FallbackEnabled = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::SDCard, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_FALSE(result.FallbackActive);
}

void test_SDCard_primary_fallback_disabled() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::SDCard;
  state.SDCardMounted = true;
  state.SDCardHasImages = false;
  state.LittleFSMounted = true;
  state.LittleFSHasImages = true;
  state.FallbackEnabled = false;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::SDCard, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_FALSE(result.FallbackActive);
}

void test_LittleFS_primary_with_images() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::LittleFS;
  state.LittleFSMounted = true;
  state.LittleFSHasImages = true;
  state.SDCardMounted = true;
  state.SDCardHasImages = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::LittleFS, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_FALSE(result.FallbackActive);
}

void test_LittleFS_primary_no_images_fallback_to_sdcard() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::LittleFS;
  state.LittleFSMounted = true;
  state.LittleFSHasImages = false;
  state.SDCardMounted = true;
  state.SDCardHasImages = true;
  state.FallbackEnabled = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::SDCard, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_TRUE(result.FallbackActive);
}

void test_LittleFS_primary_not_mounted_fallback() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::LittleFS;
  state.LittleFSMounted = false;
  state.SDCardMounted = true;
  state.SDCardHasImages = true;
  state.FallbackEnabled = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::SDCard, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_TRUE(result.FallbackActive);
}

void test_nothing_mounted() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::SDCard;
  state.SDCardMounted = false;
  state.LittleFSMounted = false;
  state.FallbackEnabled = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_FALSE(result.Mounted);
}

void test_only_fallback_mounted() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::SDCard;
  state.SDCardMounted = false;
  state.LittleFSMounted = true;
  state.LittleFSHasImages = true;
  state.FallbackEnabled = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::LittleFS, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_TRUE(result.FallbackActive);
}

void test_only_primary_mounted_no_images() {
  MockStorageState state;
  state.DefaultFS = EFileSystemType::SDCard;
  state.SDCardMounted = true;
  state.SDCardHasImages = false;
  state.LittleFSMounted = false;
  state.FallbackEnabled = true;
  StorageSelection result = SelectActiveStorage(state);
  TEST_ASSERT_EQUAL(EFileSystemType::SDCard, result.ActiveType);
  TEST_ASSERT_TRUE(result.Mounted);
  TEST_ASSERT_FALSE(result.FallbackActive);
}

void test_RenameFile_delegates_to_sdcard_when_active() {
  StorageSelection state = {EFileSystemType::SDCard, true, false};
  TEST_ASSERT_EQUAL(ERenameBackend::SDCard, SelectRenameBackend(state));
}

void test_RenameFile_delegates_to_littlefs_when_active() {
  StorageSelection state = {EFileSystemType::LittleFS, true, false};
  TEST_ASSERT_EQUAL(ERenameBackend::LittleFS, SelectRenameBackend(state));
}

void test_RenameFile_returns_none_when_not_mounted() {
  StorageSelection state = {EFileSystemType::SDCard, false, false};
  TEST_ASSERT_EQUAL(ERenameBackend::None, SelectRenameBackend(state));
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_SDCard_primary_with_images);
  RUN_TEST(test_SDCard_primary_no_images_fallback_to_littlefs);
  RUN_TEST(test_SDCard_primary_not_mounted_fallback);
  RUN_TEST(test_SDCard_primary_no_images_no_fallback_images);
  RUN_TEST(test_SDCard_primary_fallback_disabled);
  RUN_TEST(test_LittleFS_primary_with_images);
  RUN_TEST(test_LittleFS_primary_no_images_fallback_to_sdcard);
  RUN_TEST(test_LittleFS_primary_not_mounted_fallback);
  RUN_TEST(test_nothing_mounted);
  RUN_TEST(test_only_fallback_mounted);
  RUN_TEST(test_only_primary_mounted_no_images);
  RUN_TEST(test_RenameFile_delegates_to_sdcard_when_active);
  RUN_TEST(test_RenameFile_delegates_to_littlefs_when_active);
  RUN_TEST(test_RenameFile_returns_none_when_not_mounted);
  return UNITY_END();
}

