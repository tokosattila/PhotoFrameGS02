#include <App/Tone.h>

namespace App {

  Tone_ &Tone_::Instance() {
    static Tone_ tInstance;
    return tInstance;
  }

  Tone_::Tone_() {
    mMutex = xSemaphoreCreateRecursiveMutex();
  }

  Tone_::~Tone_() {
    End();
    if (mMutex) {
      vSemaphoreDelete(mMutex);
      mMutex = nullptr;
    }
  }

  void Tone_::Lock() {
    if (Instance().mMutex) xSemaphoreTakeRecursive(Instance().mMutex, portMAX_DELAY);
  }

  void Tone_::Unlock() {
    if (Instance().mMutex) xSemaphoreGiveRecursive(Instance().mMutex);
  }

  void Tone_::DelayMs(uint32_t tDurationMs) {
    if (tDurationMs == 0) return;
    TickType_t tTicks = pdMS_TO_TICKS(tDurationMs);
    if (tTicks == 0) tTicks = 1;
    vTaskDelay(tTicks);
  }

  bool Tone_::Init(uint8_t tPin, bool tVerbose) {
    Guard tLock;
    if (!mEnabled) return false;
    mPin = tPin;
    #if defined(ESP32)
      if (!mAttached) {
        pinMode(mPin, OUTPUT);
        ledcSetup(kChannel, kBaseFreq, kResolutionBits);
        ledcAttachPin(mPin, kChannel);
        ledcWrite(kChannel, 0);
        mAttached = true;
        if (tVerbose) xLOG("Tone init successful on pin %u", (unsigned)mPin);
      }
      mAvailable = true;
    #else
      (void)tVerbose;
      mAvailable = false;
    #endif
    return mAvailable;
  }

  void Tone_::End() {
    Guard tLock;
    #if defined(ESP32)
      if (mAttached) {
        ledcWrite(kChannel, 0);
        ledcWriteTone(kChannel, 0);
        ledcDetachPin(mPin);
        mAttached = false;
      }
    #endif
    mAvailable = false;
  }

  bool Tone_::PlayStep(const SToneStep &tStep) {
    #if defined(ESP32)
      if (!mAvailable || !mAttached) return false;
      if (tStep.FrequencyHz > 0 && tStep.DurationMs > 0) {
        uint8_t tDutyPct = tStep.DutyPct > 100 ? 100 : tStep.DutyPct;
        const uint32_t tMaxDuty = (1U << kResolutionBits) - 1U;
        const uint32_t tDuty = (tMaxDuty * tDutyPct) / 100U;
        ledcWriteTone(kChannel, tStep.FrequencyHz);
        ledcWrite(kChannel, tDuty);
        DelayMs(tStep.DurationMs);
        ledcWrite(kChannel, 0);
        ledcWriteTone(kChannel, 0);
      }
      if (tStep.PauseMs > 0) DelayMs(tStep.PauseMs);
      return true;
    #else
      (void)tStep;
      return false;
    #endif
  }

  bool Tone_::Play(const SToneStep *tSteps, size_t tCount) {
    Guard tLock;
    if (!mEnabled || !tSteps || tCount == 0) return false;
    if (!mAvailable && !Init(mPin)) return false;
    bool tOk = true;
    for (size_t i = 0; i < tCount; i++) {
      if (!PlayStep(tSteps[i])) {
        tOk = false;
        break;
      }
    }
    return tOk;
  }

  bool Tone_::Beep(uint16_t tFrequencyHz, uint16_t tDurationMs, uint8_t tDutyPct) {
    const SToneStep tBeep[] = {{tFrequencyHz, tDurationMs, 0, tDutyPct}};
    return Play(tBeep, sizeof(tBeep) / sizeof(tBeep[0]));
  }

}
