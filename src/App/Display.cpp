#include <App/Display.h>

namespace App {

  Display_ &Display_::Instance() {
    static Display_ tInstance;
    return tInstance;
  }

  Display_::Display_() {
    mMutex = xSemaphoreCreateRecursiveMutex();
  }

  Display_::~Display_() {
    if (mFrameBuffer) heap_caps_free(mFrameBuffer);
    if (mMutex) {
      vSemaphoreDelete(mMutex);
      mMutex = nullptr;
    }
  }

  void Display_::Lock() {
    if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
  }

  void Display_::Unlock() {
    if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
  }

  void Display_::Init() {
    ReloadConfig();
    epd_init();
    mFrameBuffer = static_cast<uint8_t*>(heap_caps_calloc(mCfg.Width * mCfg.Height / 2, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!mFrameBuffer) xLOG("Allocation PS memory failed!");
    SetFont(&OpenSans13B);
    SetBgColor(EDisplayColor::White);
    SetColor(EDisplayColor::Black);
    memset(mFrameBuffer, static_cast<uint8_t>(mBgColor), mCfg.Width * mCfg.Height / 2);
    xLOG("Display init successful!");
  }

  void Display_::ReloadConfig() {
    Guard tLock;
    mCfg = CFG.Get<SDisplayConfig>();
  }

  void Display_::DeInit() {
    OffAll();
    i2s_deinit();
  };

  void Display_::SetFont(const GFXfont *tFont) {
    mFont = tFont;
  }

  void Display_::SetBgColor(EDisplayColor tColor) {
    Guard tLock;
    mBgColor = tColor;
    memset(mFrameBuffer, static_cast<uint8_t>(tColor), mCfg.Width * mCfg.Height / 2);
  }
  
  void Display_::SetColor(EDisplayColor tColor) {
    mColor = tColor;
  }

  void Display_::On() {
    epd_poweron();
  }

  void Display_::Off() {
    epd_poweroff();
  }

  void Display_::OffAll() {
    epd_poweroff_all();
  }

  void Display_::FillRect(int32_t tX, int32_t tY, int32_t tWidth, int32_t tHeight, EDisplayColor tColor) {
    Guard tLock;
    epd_fill_rect(tX, tY, tWidth, tHeight, static_cast<uint8_t>(tColor), mFrameBuffer);
  }

  void Display_::DrawRect(int32_t tX, int32_t tY, int32_t tWidth, int32_t tHeight, EDisplayColor tColor) {
    Guard tLock;
    epd_draw_rect(tX, tY, tWidth, tHeight, static_cast<uint8_t>(tColor), mFrameBuffer);
  }

  Rect_t Display_::WriteText(int32_t tX, int32_t tY, const char *tText, EDisplayHAlignment tHAlignment, EDisplayVAlignment tVAlignment, EDisplayColor tBgColor) {
    Guard tLock;
    int32_t tCursorX = tX;
    int32_t tCursorY = tY;
    int32_t tBoundX = 0; 
    int32_t tBoundY = 0;
    int32_t tBoundWidth = 0; 
    int32_t tBoundHeight = 0;
    get_text_bounds(const_cast<GFXfont*>(mFont), tText, &tCursorX, &tCursorY, &tBoundX, &tBoundY, &tBoundWidth, &tBoundHeight, nullptr);
    if (tX == 0 && tHAlignment == EDisplayHAlignment::Center) tCursorX = mCfg.Width / 2;
    if (tY == 0 && tVAlignment == EDisplayVAlignment::Center) tCursorY = mCfg.Height / 2;
    if (tHAlignment == EDisplayHAlignment::Left) tCursorX = tX;
    if (tHAlignment == EDisplayHAlignment::Right) tCursorX = tX - tBoundWidth;
    if (tHAlignment == EDisplayHAlignment::Center) tCursorX -= tBoundWidth / 2;
    if (tVAlignment == EDisplayVAlignment::Center) tCursorY -= tBoundHeight / 2;
    tCursorX = max(0, min(tCursorX, mCfg.Width - tBoundWidth));
    tCursorY = max(0, min(tCursorY, mCfg.Height - tBoundHeight));
    tCursorY += tBoundHeight;
    FontProperties tProps{};
    tProps.fg_color = static_cast<uint8_t>(tBgColor == EDisplayColor::Black ? EDisplayColor::White : mColor);
    tProps.bg_color = static_cast<uint8_t>(tBgColor);
    DrawMode_t tMode = tBgColor == EDisplayColor::Black ? WHITE_ON_BLACK : BLACK_ON_WHITE;
    write_mode(const_cast<GFXfont*>(mFont), tText, &tCursorX, &tCursorY, mFrameBuffer, tMode, &tProps);
    return Rect_t{.x = tCursorX - tBoundWidth, .y = tCursorY - tBoundHeight, .width = tBoundWidth, .height = tBoundHeight};
  }

  void Display_::WriteTextCentered(const char *tText, EDisplayColor tBgColor) {
    WriteText(0, 0, tText, EDisplayHAlignment::Center, EDisplayVAlignment::Center, tBgColor);
  }

  void Display_::WriteTextWithBoxCentered(const char *tText, int32_t tXPadding, int32_t tYPadding, EDisplayColor tBoxColor) {
    Guard tLock;
    Rect_t tTextRect = WriteText(0, 0, tText, EDisplayHAlignment::Center, EDisplayVAlignment::Center);
    memset(mFrameBuffer, static_cast<uint8_t>(mBgColor), mCfg.Width * mCfg.Height / 2);
    int32_t tBoxX = tTextRect.x - tXPadding;
    int32_t tBoxY = tTextRect.y - tYPadding;
    int32_t tBoxW = tTextRect.width + 2 * tXPadding;
    int32_t tBoxH = tTextRect.height + 2 * tYPadding;
    tBoxX = max(0, tBoxX);
    tBoxY = max(0, tBoxY);
    tBoxW = min(tBoxW, mCfg.Width - tBoxX);
    tBoxH = min(tBoxH, mCfg.Height - tBoxY);
    FillRect(tBoxX, tBoxY, tBoxW, tBoxH, tBoxColor);
    WriteText(0, 0, tText, EDisplayHAlignment::Center, EDisplayVAlignment::Center, tBoxColor);
  }

  void Display_::PrintImage(int32_t tX, int32_t tY, int32_t tWidth, int32_t tHeight, const uint8_t *tData) {
    Rect_t tArea{.x = tX, .y = tY, .width = tWidth, .height = tHeight};
    CopyToFrameBuffer(tArea, tData);
  }

  int IRAM_ATTR Display_::JpegDrawCallback(JPEGDRAW *tDraw) {
    Display_ *tSelf = &Instance();
    if (!tSelf->mFrameBuffer) return 0;
    for (uint16_t tY = 0; tY < tDraw->iHeight; ++tY) {
      for (uint16_t tX = 0; tX < tDraw->iWidth; ++tX) {
        uint16_t tPixel = tDraw->pPixels[tY * tDraw->iWidth + tX];
        uint8_t tGray8 = tPixel & 0xFF;
        int tVal = tGray8;
        tVal = 128 + (tVal - 128) * tSelf->mCfg.JpgContrast / 100;
        tVal += (tSelf->mCfg.JpgBrightness * 255) / 100;
        if (tSelf->mCfg.JpgGamma != 100) {
          float tF = tVal / 255.0f;
          tF = powf(tF, 100.0f / tSelf->mCfg.JpgGamma);
          tVal = (int)(tF * 255.0f + 0.5f);
        }
        tVal = max(0, min(255, tVal));
        uint8_t tLevel4bit = tVal >> 4;
        uint32_t tByteOffset = (tDraw->y + tY) * (tSelf->mCfg.Width / 2) + (tDraw->x + tX) / 2;
        uint8_t tShift = ((tDraw->x + tX) & 1) * 4;
        tSelf->mFrameBuffer[tByteOffset] = (tSelf->mFrameBuffer[tByteOffset] & ~(0x0F << tShift)) | (tLevel4bit << tShift);
      }
    }
    return 1;
  }

  bool Display_::PrintJpg(int32_t tX, int32_t tY, const char *tFileName) {
    struct STaskData {
      int32_t X;
      int32_t Y;
      const char *FileName;
      SemaphoreHandle_t Done;
      bool Success;
    };
    SemaphoreHandle_t tDoneSemaphore = xSemaphoreCreateBinary();
    if (!tDoneSemaphore) {
      xLOG("Failed to create semaphore for JPEG decoding");
      return false;
    }
    STaskData *tTaskData = new STaskData{tX, tY, tFileName, tDoneSemaphore, false};
    xLOG("Decoding JPEG → %s", tFileName);
    ClearDisplay();
    xTaskCreate([](void *tParameter) {
      STaskData *tData = static_cast<STaskData*>(tParameter);
      Display_ &tSelf = Instance();
      char tFullPath[128];
      snprintf(tFullPath, sizeof(tFullPath), "/%s/%s", tSelf.mCfg.ImagesDir.c_str(), tData->FileName);
      File tFile = LFS.OpenFile(tFullPath);
      if (tFile) {
        size_t tSize = tFile.size();
        uint8_t *tBuffer = static_cast<uint8_t*>(heap_caps_malloc(tSize, MALLOC_CAP_SPIRAM));
        if (tBuffer) {
          tFile.read(tBuffer, tSize);
          tFile.close();
          JPEGDEC tJpeg;
          if (tJpeg.openRAM(tBuffer, tSize, &tSelf.JpegDrawCallback)) {
            tJpeg.setPixelType(RGB565_BIG_ENDIAN);
            int16_t tDrawX = (tData->X < 0) ? (tSelf.mCfg.Width - tJpeg.getWidth()) / 2 : tData->X;
            int16_t tDrawY = (tData->Y < 0) ? (tSelf.mCfg.Height - tJpeg.getHeight()) / 2 : tData->Y;
            {
              Guard tLock;
              tJpeg.decode(tDrawX, tDrawY, 0);
            }
            tJpeg.close();
            tSelf.Update();
            tData->Success = true;
          } else xLOG("JPEG openRAM failed!");
          heap_caps_free(tBuffer);
        } else xLOG("JPEG buffer allocation failed!");
      } else xLOG("Failed to open file for JPEG decoding → %s", tFullPath);
      xSemaphoreGive(tData->Done);
      vTaskDelete(nullptr);
    }, "JpgDecode", 32 * 1024, tTaskData, 5, nullptr);
    constexpr TickType_t kJpegTimeoutMs = 30000;
    bool tSuccess = false;
    if (xSemaphoreTake(tDoneSemaphore, pdMS_TO_TICKS(kJpegTimeoutMs)) == pdTRUE) {
      tSuccess = tTaskData->Success;
      xLOG("JPEG decode %s → %s", tSuccess ? "success" : "failed", tFileName);
    } else {
      xLOG("JPEG decode timeout → %s", tFileName);
    }
    vSemaphoreDelete(tDoneSemaphore);
    delete tTaskData;
    return tSuccess;
  }
    
  void Display_::Update() {
    Guard tLock;
    On();
    epd_draw_grayscale_image(epd_full_screen(), mFrameBuffer);
    Off();
  }

  Rect_t Display_::CopyToFrameBuffer(Rect_t tArea, const uint8_t *tData) {
    Guard tLock;
    epd_copy_to_framebuffer(tArea, const_cast<uint8_t*>(tData), mFrameBuffer);
    return tArea;
  }

  void Display_::ClearArea(int32_t tX, int32_t tY, int32_t tWidth, int32_t tHeight) {
    Rect_t tArea{.x = tX, .y = tY, .width = tWidth, .height = tHeight};
    Guard tLock;
    On();
    epd_clear_area_cycles(tArea, 4, 100);
    Off();
  }

  void Display_::ClearDisplay() {
    ClearArea(0, 0, mCfg.Width, mCfg.Height);
  }

}
