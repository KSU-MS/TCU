#pragma once

#include <stdint.h>

class freq {
private:
  uint32_t pulse_width;
  uint32_t pulse_start;
  uint32_t pulse_high;

public:
  inline float get_frequency() { return 1000000 / pulse_width; }
  inline uint32_t get_duty_cycle() { return (pulse_high / pulse_width); }

  void update(bool state);
};

#ifdef TEENSYDUINO
#include <Arduino.h>

void freq::update(bool state) {
  if (state == true) {
    // End of last pulse, total width of high and low
    pulse_width = micros() - pulse_start;

    // Start of new pulse
    pulse_start = micros();
  } else {
    // End of the High side of the new pulse
    pulse_high = micros() - pulse_start;
  }
}
#endif
