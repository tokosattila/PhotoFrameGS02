#include <App/Utils.h>

namespace App {

  RTC_DATA_ATTR bool gBootRtcReady = false;

  Utils_ &Utils_::Instance() {
    static Utils_ tInstance;
    return tInstance;
  }

  Utils_::Utils_() {
    mMutex = xSemaphoreCreateRecursiveMutex();
  }

  Utils_::~Utils_() {
    if (mMutex) {
      vSemaphoreDelete(mMutex);
      mMutex = nullptr;
    }
  }

  void Utils_::Lock() {
    if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
  }

  void Utils_::Unlock() {
    if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
  }

  bool Utils_::SecureStrcmp(const char *tA, const char *tB) {
    if (!tA || !tB) return false;
    size_t tLenA = strlen(tA);
    size_t tLenB = strlen(tB);
    volatile uint8_t tDiff = static_cast<uint8_t>(tLenA ^ tLenB);
    size_t tMinLen = (tLenA < tLenB) ? tLenA : tLenB;
    for (size_t i = 0; i < tMinLen; i++) {
      tDiff |= static_cast<uint8_t>(tA[i] ^ tB[i]);
    }
    return tDiff == 0;
  }

  uint32_t Utils_::SafeAtoul(const char *tStr, uint32_t tMinVal, uint32_t tMaxVal, uint32_t tDefaultVal) {
    if (!tStr || *tStr == '\0' || *tStr == ' ' || *tStr == '\t') return tDefaultVal;
    char *tEndPtr = nullptr;
    errno = 0;
    unsigned long tVal = strtoul(tStr, &tEndPtr, 10);
    if (errno == ERANGE || tEndPtr == tStr || *tEndPtr != '\0') return tDefaultVal;
    if (tVal < tMinVal || tVal > tMaxVal) return tDefaultVal;
    return static_cast<uint32_t>(tVal);
  }

  bool Utils_::IsSD(const char *tTarget) {
    return strcasecmp(tTarget, "sd") == 0 || strcasecmp(tTarget, "sdcard") == 0;
  }
  
  bool Utils_::IsLFS(const char *tTarget) {
    return strcasecmp(tTarget, "lfs") == 0 || strcasecmp(tTarget, "littlefs") == 0;
  }

  bool Utils_::IsValidTarget(const char *tTarget) {
    return IsSD(tTarget) || IsLFS(tTarget);
  }

  bool Utils_::IsSameTarget(const char *tA, const char *tB) {
    return (IsSD(tA) && IsSD(tB)) || (IsLFS(tA) && IsLFS(tB));
  }

  bool Utils_::GlobMatch(const char *tPattern, const char *tText) {
    while (*tPattern) {
      if (*tPattern == '*') {
        ++tPattern;
        if (!*tPattern) return true;
        while (*tText) {
          if (GlobMatch(tPattern, tText)) return true;
          ++tText;
        }
        return false;
      }
      if (tolower((unsigned char)*tPattern) != tolower((unsigned char)*tText)) return false;
      ++tPattern;
      ++tText;
    }
    return *tText == '\0';
  }

  bool Utils_::SplitPathAndFile(const char *tSpec, char *tDir, size_t tDirSize, char *tFile, size_t tFileSize) {
    if (!tSpec || *tSpec == '\0') return false;
    const char *tEnd = tSpec + strlen(tSpec);
    while (tEnd > tSpec && (*(tEnd - 1) == ' ' || *(tEnd - 1) == '\t')) --tEnd;
    if (tEnd <= tSpec) return false;
    if (*tSpec == '/') {
      const char *tLastSlash = tSpec;
      for (const char *tC = tSpec + 1; tC < tEnd; ++tC) {
        if (*tC == '/') tLastSlash = tC;
      }
      const char *tAfter = tLastSlash + 1;
      size_t tAfterLen = tEnd - tAfter;
      if (tAfterLen == 0) return false;
      size_t tDirLen = tLastSlash - tSpec;
      if (tDirLen == 0) {
        tDir[0] = '/';
        tDir[1] = '\0';
      } else {
        if (tDirLen >= tDirSize) tDirLen = tDirSize - 1;
        strncpy(tDir, tSpec, tDirLen);
        tDir[tDirLen] = '\0';
      }
      if (tAfterLen >= tFileSize) tAfterLen = tFileSize - 1;
      strncpy(tFile, tAfter, tAfterLen);
      tFile[tAfterLen] = '\0';
    } else {
      snprintf(tDir, tDirSize, "/%s", IMAGES_DIR);
      size_t tLen = tEnd - tSpec;
      if (tLen >= tFileSize) tLen = tFileSize - 1;
      strncpy(tFile, tSpec, tLen);
      tFile[tLen] = '\0';
    }
    return tFile[0] != '\0';
  }

  void Utils_::CollectMatchingFiles(const char *tDir, const char *tPattern, bool tIsSD, std::vector<String> &tFiles) {
    File tRoot = tIsSD ? SD.open(tDir) : LittleFS.open(tDir);
    if (!tRoot || !tRoot.isDirectory()) return;
    File tEntry = tRoot.openNextFile();
    while (tEntry) {
      if (!tEntry.isDirectory()) {
        const char *tName = tEntry.name();
        const char *tSlash = strrchr(tName, '/');
        if (tSlash) tName = tSlash + 1;
        if (GlobMatch(tPattern, tName)) tFiles.push_back(String(tName));
      }
      tEntry = tRoot.openNextFile();
    }
    tRoot.close();
  }

  void Utils_::ResolveFileSpec(const char *tDir, const char *tSpec, bool tIsSD, std::vector<String> &tFiles) {
    bool tHasGlob = (strchr(tSpec, '*') != nullptr);
    bool tHasComma = (strchr(tSpec, ',') != nullptr);
    if (!tHasGlob && !tHasComma) {
      tFiles.push_back(String(tSpec));
      return;
    }
    if (tHasComma) {
      char tBuf[256] = "";
      strncpy(tBuf, tSpec, sizeof(tBuf) - 1);
      char *tToken = strtok(tBuf, ",");
      while (tToken) {
        while (*tToken == ' ') ++tToken;
        if (*tToken != '\0') {
          char *tE = tToken + strlen(tToken) - 1;
          while (tE > tToken && *tE == ' ') *tE-- = '\0';
          if (strchr(tToken, '*')) CollectMatchingFiles(tDir, tToken, tIsSD, tFiles);
          else tFiles.push_back(String(tToken));
        }
        tToken = strtok(nullptr, ",");
      }
      return;
    }
    CollectMatchingFiles(tDir, tSpec, tIsSD, tFiles);
  }

  void Utils_::Init() {
    ReloadConfig();
  }

  void Utils_::ReloadConfig() {
    Guard tLock;
    mCfg = CFG.Get<SAppConfig>();
  }

  void Utils_::SetCPUFrequency(ECPUFrequency tFrequency) {
    Guard tLock;
    setCpuFrequencyMhz(static_cast<uint32_t>(tFrequency));
  }
  
  void Utils_::DisableBT() {
    Guard tLock;
    esp_bt_controller_status_t tStatus = esp_bt_controller_get_status();
    if (tStatus != ESP_BT_CONTROLLER_STATUS_IDLE) {
      if (tStatus == ESP_BT_CONTROLLER_STATUS_ENABLED) {
        if (esp_bt_controller_disable() != ESP_OK) {
          while (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED) vTaskDelay(1);
        }
      }
      if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED) esp_bt_controller_deinit();
      esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
    }
    xLOG("Bluetooth disabled");
  }

  void Utils_::DisableTouchPad() {
    Guard tLock;
    touch_pad_init();
    touch_pad_deinit();
    xLOG("Touch Pad disabled");
  }

  void Utils_::DisableBrownout() {
    Guard tLock;
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    xLOG("Brownout detector disabled");
  }

  void Utils_::ByteToReadableSize(uint64_t tBytes, char *tBuffer, size_t tLength) {
    if (tBytes < 1024ULL) {
      snprintf(tBuffer, tLength, "%llu B", static_cast<unsigned long long>(tBytes));
    } else if (tBytes < 1024ULL * 1024ULL) {
      float tSizeKB = static_cast<float>(tBytes) / 1024.0f;
      if (fabs(tSizeKB - static_cast<int>(tSizeKB)) < 0.01f) snprintf(tBuffer, tLength, "%d KB", static_cast<int>(tSizeKB));
      else snprintf(tBuffer, tLength, "%.2f KB", tSizeKB);
    } else if (tBytes < 1024ULL * 1024ULL * 1024ULL) {
      float tSizeMB = static_cast<float>(tBytes) / (1024.0f * 1024.0f);
      if (fabs(tSizeMB - static_cast<int>(tSizeMB)) < 0.01f) snprintf(tBuffer, tLength, "%d MB", static_cast<int>(tSizeMB));
      else snprintf(tBuffer, tLength, "%.2f MB", tSizeMB);
    } else {
      float tSizeGB = static_cast<float>(tBytes) / (1024.0f * 1024.0f * 1024.0f);
      if (fabs(tSizeGB - static_cast<int>(tSizeGB)) < 0.01f) snprintf(tBuffer, tLength, "%d GB", static_cast<int>(tSizeGB));
      else snprintf(tBuffer, tLength, "%.2f GB", tSizeGB);
    }
  }

  const char *Utils_::EpochToReadableFormat(unsigned long tEpoch, bool tAsDateTime, char *tBuffer, size_t tLength) {
    if (!tBuffer || tLength == 0) return "";
    tBuffer[0] = '\0';
    if (tEpoch == 0) {
      strcpy(tBuffer, "0");
      return tBuffer;
    }
    if (tAsDateTime) {
      time_t tTime = (time_t)(tEpoch);
      struct tm tTm;
      localtime_r(&tTime, &tTm);
      snprintf(tBuffer, tLength, "%04d.%02d.%02d %02d:%02d:%02d", tTm.tm_year + 1900, tTm.tm_mon + 1, tTm.tm_mday, tTm.tm_hour, tTm.tm_min, tTm.tm_sec);
      return tBuffer;
    }
    if (tEpoch < SECONDS_PER_MINUTE) snprintf(tBuffer, tLength, "%lu sec", tEpoch);
    else if (tEpoch < SECONDS_PER_HOUR) snprintf(tBuffer, tLength, "%02lu:%02lu min", tEpoch / SECONDS_PER_MINUTE, tEpoch % SECONDS_PER_MINUTE);
    else if (tEpoch < SECONDS_PER_DAY) snprintf(tBuffer, tLength, "%lu:%02lu:%02lu hour(s)", tEpoch / SECONDS_PER_HOUR, (tEpoch % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE, tEpoch % SECONDS_PER_MINUTE);
    else snprintf(tBuffer, tLength, "%lu day(s) %02lu:%02lu:%02lu", tEpoch / SECONDS_PER_DAY, (tEpoch % SECONDS_PER_DAY) / SECONDS_PER_HOUR, (tEpoch % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE, tEpoch % SECONDS_PER_MINUTE);
    return tBuffer;
  }

  void Utils_::PrintPartitionInfo() {
    char tText[mPrintInfoWidth - 4] = "";
    const esp_partition_t *tRunning = esp_ota_get_running_partition();
    const esp_partition_t *tBoot = esp_ota_get_boot_partition();
    PrintInfo("PARTITION INFO", EUtilsInfoType::Header);
    PrintInfo("", EUtilsInfoType::Line);
    if (tRunning) {
      snprintf(tText, sizeof(tText), "Running partition: %s @ 0x%08x", tRunning->label, (unsigned)tRunning->address);
      PrintInfo(tText);
    }
    if (tBoot) {
      snprintf(tText, sizeof(tText), "Boot partition: %s @ 0x%08x", tBoot->label, (unsigned)tBoot->address);
      PrintInfo(tText);
    }
    PrintInfo("", EUtilsInfoType::Footer);
  }

  void Utils_::PrintBootInfo() {
    if (gBootCount == 0) xLOG("First boot [%d]\n", gBootCount);
    else {     
      xLOG("Boot no. → %d", gBootCount);
      PrintWakeupReason();
      xLOG_PL();
    }
  }

  void Utils_::PrintDeviceInfo() {
    Guard tLock;
    char tText[mPrintInfoWidth - 4] = "";
    xLOG_PL();
    PrintBootInfo();
    snprintf(tText, sizeof(tText), "%s %s", mCfg.Device.Name.c_str(), mCfg.Device.Version.c_str());
    PrintInfo(tText, EUtilsInfoType::Header);
    PrintChipInfo();
    PrintRamInfo();
    PrintFlashInfo();
    PrintResourceInfo();
    PrintRadioInfo();
    PrintInfo("", EUtilsInfoType::Footer);
  }

  void Utils_::PrintInfo(const char *tText, EUtilsInfoType tType, uint8_t tWidth) {
    if (tWidth == 0) tWidth = mPrintInfoWidth;
    tWidth -= 2;
    if (tType == EUtilsInfoType::Header || tType == EUtilsInfoType::Single) {
      if (tType == EUtilsInfoType::Single) xLOG_PR_("\n┌─");
      else xLOG_PR_("┌─");
      for (uint8_t i = 0; i < tWidth; i++) xLOG_PR_("─");
      xLOG_PL_("─┐");
    } else 
    if (tType == EUtilsInfoType::Title) {
      xLOG_PR_("├─");
      for (uint8_t i = 0; i < tWidth; i++) xLOG_PR_("─");
      xLOG_PL_("─┤");
    }
    if (tType == EUtilsInfoType::Line) {
      xLOG_PR_("├─");
      for (uint8_t i = 0; i < tWidth; i++) xLOG_PR_("─");
      xLOG_PL_("─┤");
    }
    uint8_t tTextLen = 0;
    for (const char *tPtr = tText; *tPtr; ++tPtr) {
      if ((*tPtr & 0xC0) != 0x80) ++tTextLen;
    }
    if (tTextLen > 0) {
      xLOG_PR_("│ ");
      uint8_t tRightPadding = (tTextLen < tWidth) ? (tWidth - tTextLen) : 0;
      if (tType == EUtilsInfoType::Header || tType == EUtilsInfoType::Title || tType == EUtilsInfoType::Single) {
        xLOG_PR(tText);
        if (tRightPadding > 1) {
          xLOG_PR(" ");
          tRightPadding -= 1;
        }
        if (tRightPadding != 0) {
          for (uint8_t i = 0; i < tRightPadding; i++) xLOG_PR_("░");
        }
      } else {
        xLOG_PR(tText);
        if (tRightPadding != 0) {
          for (uint8_t i = 0; i < tRightPadding; i++) xLOG_PR(" ");
        }
      }
      xLOG_PL_(" │");
    }
    if (tType == EUtilsInfoType::Title) {
      xLOG_PR_("├─");
      for (uint8_t i = 0; i < tWidth; i++) xLOG_PR_("─");
      xLOG_PL_("─┤");
    } else if (tType == EUtilsInfoType::Footer || tType == EUtilsInfoType::Single) {
      xLOG_PR_("└─");
      for (uint8_t i = 0; i < tWidth; i++) xLOG_PR_("─");
      xLOG_PL_("─┘\n");
    }
  }

  void Utils_::PrintChipInfo() {
    char tText[mPrintInfoWidth - 4] = "";
    esp_chip_info_t tChip;
    esp_chip_info(&tChip);
    PrintInfo("CHIP INFO", EUtilsInfoType::Title);
    snprintf(tText, sizeof(tText), "Type: %s, rev. %d.%d", ESP.getChipModel(), tChip.revision / 100, tChip.revision % 100);
    PrintInfo(tText);
    snprintf(tText, sizeof(tText), "Cores: %d", tChip.cores);
    PrintInfo(tText);
    snprintf(tText, sizeof(tText), "Frequency: %d MHz", ESP.getCpuFreqMHz());
    PrintInfo(tText);
    snprintf(tText, sizeof(tText), "Temperature: %.1f °C", temperatureRead());
    PrintInfo(tText, EUtilsInfoType::Cell, mPrintInfoWidth);
  }

  void Utils_::PrintFlashInfo() {
    char tText[19] = "";
    char tFlasChipSize[16];
    PrintInfo("FLASH INFO",EUtilsInfoType::Title);
    snprintf(tText, sizeof(tText), "Frequency: %d MHz", ESP.getFlashChipSpeed() / 1000000);
    PrintInfo(tText);
    ByteToReadableSize(ESP.getFlashChipSize(), tFlasChipSize, sizeof(tFlasChipSize));
    snprintf(tText, sizeof(tText), "Size: %s", tFlasChipSize);
    PrintInfo(tText);    
  }

  void Utils_::PrintNvsUsageInfo() {
    nvs_stats_t tStats{};
    if (nvs_get_stats(nullptr, &tStats) != ESP_OK) {
      PrintInfo("NVS: NVS failed");
      return;
    }
    size_t tUsedCfg = 0;
    nvs_iterator_t tIt = nvs_entry_find("nvs", "cfg", NVS_TYPE_ANY);
    while (tIt != nullptr) {
      ++tUsedCfg;
      tIt = nvs_entry_next(tIt);
    }
    nvs_release_iterator(tIt);
    char tText[128] = "";
    snprintf(tText, sizeof(tText), "NVS: %zu / %zu entries", tUsedCfg, tStats.total_entries);
    PrintInfo(tText);
  }

  void Utils_::PrintRamInfo() {
    char tText[17] = "";
    uint32_t tTotalDram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    uint32_t tTotalIram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_EXEC);
    uint32_t tTotalRam = tTotalDram + tTotalIram;
    char tTotalDramBuffer[17] = "";
    char tTotalIramBuffer[17] = "";
    char tTotalRamBuffer[17] = "";   
    PrintInfo("RAM INFO", EUtilsInfoType::Title);
    ByteToReadableSize(tTotalDram, tTotalDramBuffer, sizeof(tTotalDramBuffer));
    snprintf(tText, sizeof(tText), "DRAM: %s", tTotalDramBuffer);
    PrintInfo(tText);
    ByteToReadableSize(tTotalIram, tTotalIramBuffer, sizeof(tTotalIramBuffer));
    snprintf(tText, sizeof(tText), "IRAM: %s", tTotalIramBuffer);
    PrintInfo(tText);
    ByteToReadableSize(tTotalRam, tTotalRamBuffer, sizeof(tTotalRamBuffer));
    snprintf(tText, sizeof(tText), "Total: %s", tTotalRamBuffer);
    PrintInfo(tText);
  }

  void Utils_::PrintDRamUsageInfo() {
    char tText[28] = "";
    uint32_t tTotalDram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    uint32_t tFreeDram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    uint32_t tUsedDram = tTotalDram - tFreeDram;
    char tUsedDramBuffer[16];
    char tTotalDramBuffer[16];
    ByteToReadableSize(tUsedDram, tUsedDramBuffer, sizeof(tUsedDramBuffer));
    ByteToReadableSize(tTotalDram, tTotalDramBuffer, sizeof(tTotalDramBuffer));
    snprintf(tText, sizeof(tText), "DRAM: %s / %s", tUsedDramBuffer, tTotalDramBuffer);
    PrintInfo(tText);
  }

  void Utils_::PrintIRamUsageInfo() {
    char tText[28] = "";
    uint32_t tTotalIram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_EXEC);
    uint32_t tFreeIram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_EXEC);
    uint32_t tUsedIram = tTotalIram - tFreeIram;
    char tUsedIramBuffer[16];
    char tTotalIramBuffer[16];
    ByteToReadableSize(tUsedIram, tUsedIramBuffer, sizeof(tUsedIramBuffer));
    ByteToReadableSize(tTotalIram, tTotalIramBuffer, sizeof(tTotalIramBuffer));
    snprintf(tText, sizeof(tText), "IRAM: %s / %s", tUsedIramBuffer, tTotalIramBuffer);
    PrintInfo(tText);
  }

  void Utils_::PrintPSRamInfo() {
    char tText[mPrintInfoWidth - 4] = "";
    uint32_t tTotalPsram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    char tTotalPsramBuffer[16] = "";
    ByteToReadableSize(tTotalPsram, tTotalPsramBuffer, sizeof(tTotalPsramBuffer));
    PrintInfo("PSRAM INFO", EUtilsInfoType::Title);
    snprintf(tText, sizeof(tText), "Size: %s", tTotalPsramBuffer);
    PrintInfo(tText);
  }

  void Utils_::PrintPSRamUsageInfo() {
    char tText[27] = "";
    uint32_t tTotalPsram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    uint32_t tFreePsram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    uint32_t tUsedPsram = tTotalPsram - tFreePsram;
    char tUsedPsramBuffer[16];
    char tTotalPsramBuffer[16];
    ByteToReadableSize(tUsedPsram, tUsedPsramBuffer, sizeof(tUsedPsramBuffer));
    ByteToReadableSize(tTotalPsram, tTotalPsramBuffer, sizeof(tTotalPsramBuffer));
    snprintf(tText, sizeof(tText), "PSRAM: %s / %s", tUsedPsramBuffer, tTotalPsramBuffer);
    PrintInfo(tText);
  }

  void Utils_::PrintMemoryInfo() {
    Guard tLock;
    char tText[mPrintInfoWidth - 4] = "";
    char tFreeHeap[16] = "";
    char tLargestFreeBlock[16] = "";
    char tMinimumFreeSize[16] = "";
    xLOG_PL();
    PrintInfo("MEMORY INFO", EUtilsInfoType::Header);
    PrintInfo("", EUtilsInfoType::Line);
    ByteToReadableSize(ESP.getFreeHeap(), tFreeHeap, sizeof(tFreeHeap));
    snprintf(tText, sizeof(tText), "Free heap: %s", tFreeHeap);
    PrintInfo(tText);
    PrintInfo("", EUtilsInfoType::Footer);
  }

  void Utils_::PrintSketchInfo() {
    char tText[30] = "";
    uint32_t tSketchSize = ESP.getSketchSize();
    const esp_partition_t *tRunning = esp_ota_get_running_partition();
    if (!tRunning) {
      PrintInfo("Sketch: unknown");
      return;
    }
    char tSketchSizeBuffer[16] = "";
    char tSketchTotalSizeBuffer[16] = "";
    ByteToReadableSize(tSketchSize, tSketchSizeBuffer, sizeof(tSketchSizeBuffer));
    ByteToReadableSize(tRunning->size, tSketchTotalSizeBuffer, sizeof(tSketchTotalSizeBuffer));
    snprintf(tText, sizeof(tText), "Sketch: %s / %s", tSketchSizeBuffer, tSketchTotalSizeBuffer);
    PrintInfo(tText);
  }

  void Utils_::PrintFileSystemInfo() {
    char tText[48] = "";
    char tUsedBuffer[16] = "";
    char tTotalBuffer[16] = "";
    if (LFS.Init(false)) {
      ByteToReadableSize(LFS.UsedBytes(), tUsedBuffer, sizeof(tUsedBuffer));
      ByteToReadableSize(LFS.TotalBytes(), tTotalBuffer, sizeof(tTotalBuffer));
      snprintf(tText, sizeof(tText), "LittleFS: %s / %s", tUsedBuffer, tTotalBuffer);
      PrintInfo(tText);
      LFS.End();
    }
    if (SDC.Init(false)) {
      ByteToReadableSize(SDC.UsedBytes(), tUsedBuffer, sizeof(tUsedBuffer));
      ByteToReadableSize(SDC.TotalBytes(), tTotalBuffer, sizeof(tTotalBuffer));
      snprintf(tText, sizeof(tText), "SDCard (%s): %s / %s", SDC.CardTypeName(), tUsedBuffer, tTotalBuffer);
      PrintInfo(tText);
      SDC.End();
    }
  }

  void Utils_::PrintResourceInfo() {
    PrintInfo("RESOURCE INFO", EUtilsInfoType::Title);
    PrintDRamUsageInfo();
    PrintIRamUsageInfo();
    PrintPSRamUsageInfo();
    PrintInfo("", EUtilsInfoType::Line);
    PrintNvsUsageInfo();
    PrintSketchInfo();
    PrintFileSystemInfo();
  }

  void Utils_::PrintRadioInfo() {
    char tText[16] = "";
    esp_chip_info_t tChip;
    esp_chip_info(&tChip);
    PrintInfo("COM INFO", EUtilsInfoType::Title);
    snprintf(tText, sizeof(tText), "WIFI: %s", (tChip.features & CHIP_FEATURE_WIFI_BGN) ? "yes" : "no");
    PrintInfo(tText);
    snprintf(tText, sizeof(tText), "BT: %s", (tChip.features & CHIP_FEATURE_BT) ? "yes" : "no");
    PrintInfo(tText);
  }

  void Utils_::PrintDateTime() {
    char tBuf[24];
    xLOG("Date/Time → %s", EpochToReadableFormat(time(nullptr), true, tBuf, sizeof(tBuf)));
  }

  const char *Utils_::PrependSlash(const char *tPath, char *tOutBuffer, size_t tBufSize) {
    if (!tPath || !tOutBuffer || tBufSize < 2) return tPath;
    size_t tLen = 0;
    while (tPath[tLen] && tLen < tBufSize-2) tLen++;
    if (tLen > 0 && tPath[0] != '/' && tPath[0] != '\\') {
      tOutBuffer[0] = '/';
      memcpy(tOutBuffer + 1, tPath, tLen);
      tOutBuffer[tLen + 1] = 0;
    } else {
      strncpy(tOutBuffer, tPath, tBufSize-1);
      tOutBuffer[tBufSize-1] = 0;
    }
    return tOutBuffer; 
  }

  void Utils_::PrintWakeupReason() {
    static const char *tReasons[] = { "RESTART", "ALL", "EXT0 (RTC_IO)", "EXT1 (RTC_CNTL)", "TIMER", "TOUCHPAD", "ULP", "GPIO", "UART", "WIFI", "COCPU", "COCPU_TRAP_TRIG", "BT" };
    esp_sleep_wakeup_cause_t tCause = esp_sleep_get_wakeup_cause();
    const char *tReasonStr = "UNKNOWN";
    if (tCause < sizeof(tReasons) / sizeof(tReasons[0])) tReasonStr = tReasons[tCause];
    xLOG("Wake-up system → %s [%d]", tReasonStr, (unsigned long)tCause);
  }

  bool Utils_::WasWokenByButton() {
    const uint8_t tConfiguredSettingPin = static_cast<uint8_t>(Instance().mCfg.Device.SettingPin);
    if (WasWokenByPin(tConfiguredSettingPin)) return true;
    return WasWokenByPin(SETTING_PIN);
  }

  bool Utils_::WasWokenByRtcAlarm() {
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_EXT1) return false;
    const uint64_t tMask = esp_sleep_get_ext1_wakeup_status();
    return (tMask & (1ULL << RTC_INT_PIN)) != 0;
  }

  void Utils_::SetBootRtcReady(bool tReady) {
    gBootRtcReady = tReady;
  }

  bool Utils_::WasBootRtcReady() {
    return gBootRtcReady;
  }

  bool Utils_::HasElapsedMs(uint32_t tStart, uint32_t tNow, uint32_t tDelayMs) {
    if (tNow >= tStart) return (tNow - tStart) >= tDelayMs;
    else return ((UINT32_MAX - tStart) + tNow + 1) >= tDelayMs;
  }

  const char *Utils_::ResolveBootReason() {
    esp_reset_reason_t tReason = esp_reset_reason();
    switch (tReason) {
      case ESP_RST_UNKNOWN: return "Unknown";
      case ESP_RST_POWERON: return "Power on";
      case ESP_RST_EXT: return "External (NRST)";
      case ESP_RST_SW: return "Software (esp_restart)";
      case ESP_RST_PANIC: return "Panic/Exception";
      case ESP_RST_INT_WDT: return "Interrupt watchdog";
      case ESP_RST_TASK_WDT: return "Task watchdog";
      case ESP_RST_WDT: return "Watchdog";
      case ESP_RST_DEEPSLEEP: return "Deep sleep";
      case ESP_RST_BROWNOUT: return "Brownout";
      case ESP_RST_SDIO: return "SDIO";
      default: return "Other";
    }
  }

  bool Utils_::WasWokenByPin(uint8_t tPin) {
    esp_sleep_wakeup_cause_t tCause = esp_sleep_get_wakeup_cause();
    if (tCause != ESP_SLEEP_WAKEUP_EXT0 && tCause != ESP_SLEEP_WAKEUP_EXT1) {
      return false;
    }
    if (tCause == ESP_SLEEP_WAKEUP_EXT1) {
      uint64_t tMask = esp_sleep_get_ext1_wakeup_status();
      if (tPin < 64 && (tMask & (1ULL << tPin))) return true;
    }
    return false;
  }

  uint64_t Utils_::SecondsUntilHour(uint8_t tTargetHour) {
    time_t tEpochUtc = time(nullptr);
    if (static_cast<uint32_t>(tEpochUtc) < 1735689600UL) return SECONDS_PER_DAY;
    struct tm tLocal;
    localtime_r(&tEpochUtc, &tLocal);
    uint32_t tNowSec = tLocal.tm_hour * SECONDS_PER_HOUR + tLocal.tm_min  * SECONDS_PER_MINUTE + tLocal.tm_sec;
    uint32_t tTargetSec = tTargetHour * SECONDS_PER_HOUR;
    if (tTargetSec <= tNowSec) tTargetSec += SECONDS_PER_DAY;
    return tTargetSec - tNowSec;
  }

  bool Utils_::TryResolveScheduleNow(SRTCDateTime &tNow) {
    constexpr unsigned long kMinValidEpoch = 1735689600UL;
    bool tNowValid = false;
    if (RTC.Init(false) && RTC.IsAvailable()) {
      tNowValid = RTC.GetDateTime(tNow);
      RTC.End();
    }
    if (!tNowValid) {
      const time_t tEpoch = time(nullptr);
      if (static_cast<unsigned long>(tEpoch) >= kMinValidEpoch) {
        RTC_::EpochToDateTime(static_cast<unsigned long>(tEpoch), tNow);
        tNowValid = true;
      }
    }
    return tNowValid;
  }

  uint32_t Utils_::ComputeFallbackDelaySeconds(const STimerConfig &tTimerCfg) {
    const uint8_t tHour = tTimerCfg.WakeUpHour % 24;
    switch (tTimerCfg.WakeUp) {
      case ETimerWakeUp::Minutes:
        return SECONDS_PER_MINUTE;
      case ETimerWakeUp::Hourly:
        return SECONDS_PER_HOUR;
      case ETimerWakeUp::HalfDay:
        return 12 * SECONDS_PER_HOUR;
      case ETimerWakeUp::Daily:
        return static_cast<uint32_t>(UTL.SecondsUntilHour(tHour));
      case ETimerWakeUp::Weekly:
        return static_cast<uint32_t>(UTL.SecondsUntilHour(tHour) + 6 * SECONDS_PER_DAY);
      case ETimerWakeUp::Monthly:
        return static_cast<uint32_t>(UTL.SecondsUntilHour(tHour) + 29 * SECONDS_PER_DAY);
      default:
        return static_cast<uint32_t>(UTL.SecondsUntilHour(tHour));
    }
  }

  bool Utils_::ComputeWakePlan(const STimerConfig &tTimerCfg, SWakeSchedule &tOutPlan) {
    SRTCDateTime tNow {};
    if (!TryResolveScheduleNow(tNow)) return false;
    return WSC.Compute(tTimerCfg, tNow, tOutPlan);
  }

  uint32_t Utils_::ComputeWakeDelaySeconds(const STimerConfig &tTimerCfg) {
    SWakeSchedule tPlan {};
    if (ComputeWakePlan(tTimerCfg, tPlan)) return tPlan.DelaySeconds;
    return ComputeFallbackDelaySeconds(tTimerCfg);
  }

  void Utils_::SleepAndWakeup() {
    constexpr uint64_t tSecToUs = 1000000ULL;
    const uint8_t tHour = mCfg.Timer.WakeUpHour % 24;
    SWakeSchedule tPlan {};
    const bool tPlanValid = ComputeWakePlan(mCfg.Timer, tPlan);
    uint64_t tDelaySec = tPlanValid ? tPlan.DelaySeconds : ComputeFallbackDelaySeconds(mCfg.Timer);
    const char *tUnit = "sec";
    uint64_t tDisplay = tDelaySec;
    if (tDisplay >= 7 * SECONDS_PER_DAY) { 
      tDisplay /= SECONDS_PER_DAY; 
      tUnit = "day";   
    } else 
    if (tDisplay >= SECONDS_PER_DAY) { 
      tDisplay /= SECONDS_PER_DAY; 
      tUnit = "day"; 
    } else 
    if (tDisplay >= SECONDS_PER_HOUR) { 
      tDisplay /= SECONDS_PER_HOUR;  
      tUnit = "hour"; 
    } else 
    if (tDisplay >= SECONDS_PER_MINUTE) { 
      tDisplay /= SECONDS_PER_MINUTE;
      tUnit = "min";
    }
    xLOG("Going to deep sleep...");
    if (tPlanValid) {
      xLOG("Wake plan → %04u-%02u-%02u %02u:%02u:%02u", tPlan.NextWake.Year, tPlan.NextWake.Month, tPlan.NextWake.Day, tPlan.NextWake.Hour, tPlan.NextWake.Minute, tPlan.NextWake.Second);
    } else {
      const bool tUsesWallClock = (mCfg.Timer.WakeUp == ETimerWakeUp::Daily || mCfg.Timer.WakeUp == ETimerWakeUp::Weekly || mCfg.Timer.WakeUp == ETimerWakeUp::Monthly);
      if (tUsesWallClock) {
        const bool tValidTime = (static_cast<uint32_t>(time(nullptr)) >= 1735689600UL);
        if (tValidTime) xLOG("Wake-up hour → %02u:00", tHour);
        else xLOG("Wake-up hour → %02u:00 (no valid time, relative fallback)", tHour);
      }
    }
    bool tAlarmArmed = false;
    if (RTC.Init(false) && RTC.IsAvailable()) {
      SetBootRtcReady(true);
      if (tPlanValid) {
        RTC.ClearAlarmFlag();
        tAlarmArmed = RTC.SetAlarm(tPlan.Alarm);
      } else {
        RTC.DisableAlarm();
        RTC.ClearAlarmFlag();
      }
      if (!tAlarmArmed && tPlanValid) RTC.DisableAlarm();
      RTC.End();
    } else {
      SetBootRtcReady(false);
    }
    xLOG("Wake source → %s", tAlarmArmed ? "RTC alarm + timer fallback" : "ESP32 timer");
    xLOG("Next wake-up → %llu %s\n\n", tDisplay, tUnit);
    uint8_t tSettingPin = static_cast<uint8_t>(mCfg.Device.SettingPin);
    uint64_t tExt1Mask = 0;
    if (rtc_gpio_is_valid_gpio(static_cast<gpio_num_t>(tSettingPin))) tExt1Mask |= (1ULL << tSettingPin);
    if (rtc_gpio_is_valid_gpio(static_cast<gpio_num_t>(SETTING_PIN))) tExt1Mask |= (1ULL << SETTING_PIN);
    if (rtc_gpio_is_valid_gpio(static_cast<gpio_num_t>(RTC_INT_PIN))) tExt1Mask |= (1ULL << RTC_INT_PIN);
    if (tExt1Mask != 0) esp_sleep_enable_ext1_wakeup(tExt1Mask, ESP_EXT1_WAKEUP_ANY_LOW);
    uint64_t tTimerSec = tDelaySec;
    if (tAlarmArmed) tTimerSec += (tDelaySec / 20) + 30;
    esp_sleep_enable_timer_wakeup(tTimerSec * tSecToUs);
    esp_deep_sleep_start();
  }  

  void Utils_::SleepLowBattery() {
    xLOG("Low battery entering deep sleep..\n\n");
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_deep_sleep_start();
  }

  bool Utils_::MeasureBattery() {
    epd_poweron();
    vTaskDelay(pdMS_TO_TICKS(50));
    uint32_t tAdcSum = 0;
    constexpr int kSamples = 16;
    for (int i = 0; i < kSamples; ++i) {
      tAdcSum += analogRead(BATTERY_PIN);
      vTaskDelay(pdMS_TO_TICKS(5));
    }
    uint32_t tAdcRaw = tAdcSum / kSamples;
    epd_poweroff();
    constexpr float kVRef = 1100.0f;
    constexpr float kAdcMax = 4095.0f;
    mBatteryVoltage = (static_cast<float>(tAdcRaw) / kAdcMax) * 2.0f * 3.3f * (kVRef / 1000.0f);
    constexpr float kVoltageTable[] = {3.20f, 3.40f, 3.50f, 3.60f, 3.70f, 3.80f, 3.90f, 4.00f, 4.10f, 4.20f};
    constexpr int kPercentTable[] = {0, 5, 10, 20, 40, 60, 75, 85, 95, 100};
    constexpr int kTableSize = sizeof(kVoltageTable) / sizeof(kVoltageTable[0]);
    if (mBatteryVoltage >= 4.20f) mBatteryPercentage = 100;
    else if (mBatteryVoltage <= 3.20f) mBatteryPercentage = 0;
    else {
      for (int i = 1; i < kTableSize; ++i) {
        if (mBatteryVoltage <= kVoltageTable[i]) {
          float tRatio = (mBatteryVoltage - kVoltageTable[i - 1]) / (kVoltageTable[i] - kVoltageTable[i - 1]);
          mBatteryPercentage = kPercentTable[i - 1] + static_cast<int>(tRatio * (kPercentTable[i] - kPercentTable[i - 1]));
          break;
        }
      }
    }
    xLOG("Battery → %.2fV [%d%%]", mBatteryVoltage, mBatteryPercentage);
    return mBatteryPercentage > 0;
  }

}
