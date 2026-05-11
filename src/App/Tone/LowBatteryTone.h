#ifndef LOW_BATTERY_TONE_H
#define LOW_BATTERY_TONE_H

namespace App {

  static constexpr SToneStep kLowBatteryTone[] = {
    {620, 260, 90, 60},
    {540, 260, 90, 60},
    {460, 360, 0, 65}
  };

}

#endif
