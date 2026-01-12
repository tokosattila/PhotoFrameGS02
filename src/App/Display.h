#ifndef DISPLAY_H
#define DISPLAY_H

#include <App/Global.h>

namespace App {

  enum class EDisplayColor : uint8_t {
    White = 0xFF,
    //Gray1 = 0xEE,
    LightGray = 0xDD,
    //Gray3 = 0xCC,
    //Gray4 = 0xBB,
    //Gray5 = 0xAA,
    Gray = 0x99,
    //Gray7 = 0x88,
    //Gray8 = 0x77,
    //Gray9 = 0x66,
    DarkGray = 0x55,
    //Gray11 = 0x44,
    //Gray12 = 0x33,
    //Gray13 = 0x22,
    //Gray14 = 0x11,
    Black = 0x00
  };

  enum class EDisplayHAlignment : uint8_t {
    Left = 1,
    Right,
    Center
  };

  enum class EDisplayVAlignment : uint8_t {
    Auto = 1,
    Center
  };

  class Display_ {
    DEFINE_TAG("DSP");
    friend class AutoGuard<Display_>;
    public:
      using Guard = AutoGuard<Display_>;
      static Display_ &Instance();
      void Init();
      void ReloadConfig();
      void DeInit();
      void SetFont(const GFXfont *tFont);
      void SetBgColor(EDisplayColor tColor);
      void SetColor(EDisplayColor tColor);
      void On();
      void Off();
      void OffAll();
      void FillRect(int32_t tX, int32_t tY, int32_t tWidth, int32_t tHeight, EDisplayColor tColor);
      void DrawRect(int32_t tX, int32_t tY, int32_t tWidth, int32_t tHeight, EDisplayColor tColor);
      Rect_t WriteText(int32_t tX, int32_t tY, const char *tText, EDisplayHAlignment tHAlignment = EDisplayHAlignment::Left, EDisplayVAlignment tVAlignment = EDisplayVAlignment::Auto, EDisplayColor tBgColor = EDisplayColor::White);
      void WriteTextCentered(const char *tText, EDisplayColor tBgColor = EDisplayColor::White);
      void WriteTextWithBoxCentered(const char *tText, int32_t tXPadding = 12, int32_t tYPadding = 8, EDisplayColor tBoxColor = EDisplayColor::Black);
      void PrintImage(int32_t tX, int32_t tY, int32_t tWidth, int32_t tHeight, const uint8_t *tData);
      bool PrintJpg(int32_t tX, int32_t tY, const char *tFileName);
      Rect_t CopyToFrameBuffer(Rect_t tArea, const uint8_t *tData);
      void Update();
      void ClearArea(int32_t tX, int32_t tY, int32_t tWidth, int32_t tHeight);
      void ClearDisplay();
    private:
      Display_();
      Display_(const Display_&) = delete;
      Display_ &operator=(const Display_&) = delete;
      ~Display_();
      mutable SemaphoreHandle_t mMutex = nullptr;
      SDisplayConfig mCfg {};
      EDisplayColor mBgColor = EDisplayColor::White;
      EDisplayColor mColor = EDisplayColor::Black;
      const GFXfont *mFont = nullptr;
      uint8_t *mFrameBuffer = nullptr;
      static void Lock();
      static void Unlock();
      static int JpegDrawCallback(JPEGDRAW *tDraw);
  };

}

#endif
