#ifndef TONE_H
#define TONE_H

#include <App/Global.h>

namespace App {

  class Tone_ {
    DEFINE_TAG("TON");
    friend class AutoGuard<Tone_>;
    public:
      using Guard = AutoGuard<Tone_>;
      static Tone_ &Instance();
      bool Init(uint8_t tPin = TONE_PIN, bool tVerbose = false);
      void End();
      bool IsAvailable() const { return mAvailable; }
      void SetEnabled(bool tEnabled) { mEnabled = tEnabled; }
      template <size_t N>
      bool Play(const SToneStep (&tSteps)[N]) { return Play(tSteps, N); }
      bool Beep(uint16_t tFrequencyHz = 1800, uint16_t tDurationMs = 80, uint8_t tDutyPct = 50);
    private:
      Tone_();
      Tone_(const Tone_&) = delete;
      Tone_ &operator=(const Tone_&) = delete;
      ~Tone_();
      static void Lock();
      static void Unlock();
      bool Play(const SToneStep *tSteps, size_t tCount);
      bool PlayStep(const SToneStep &tStep);
      static void DelayMs(uint32_t tDurationMs);
      mutable SemaphoreHandle_t mMutex = nullptr;
      uint8_t mPin = TONE_PIN;
      bool mAvailable = false;
      bool mAttached = false;
      bool mEnabled = true;
      static constexpr uint8_t kChannel = 2;
      static constexpr uint8_t kResolutionBits = 10;
      static constexpr uint32_t kBaseFreq = 5 * 1000;
  };

}

#endif
