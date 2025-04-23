#pragma once

#include <stdint.h>

class freq {
private:
  uint32_t pulse_width;
  uint32_t pulse_start;
  uint32_t pulse_high;
  uint32_t attempts = 0;
  bool last_state = false;

public:
  inline double get_frequency() { return 1000.0 / pulse_width; }
  inline double get_duty_cycle() { return pulse_high / pulse_width; }

  void update(bool state);
};

#ifdef TEENSYDUINO
#include <Arduino.h>

void freq::update(bool state) {
  if (state == true && last_state == false) {
    // End of last pulse, total width of high and low
    pulse_width = millis() - pulse_start;

    // Start of new pulse
    pulse_start = millis();

    attempts = 0;
    last_state = true;
  } else if (state == false && last_state == true) {
    // End of the High side of the new pulse
    pulse_high = millis() - pulse_start;

    last_state = false;
  } else if (attempts > 100000) {
    pulse_width = 100000;
  }

  attempts++;
}
#endif
