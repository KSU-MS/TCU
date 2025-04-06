#include "main.hpp"
#include "core_pins.h"

void setup() {
  pinMode(FREQ_ACCU_PIN, INPUT);
  pinMode(FREQ_TS_PIN, INPUT);
  pinMode(PWR_OK_PIN, INPUT);

  Serial.begin(9600);
  Serial.println("Entering STANDBY");
}

void loop() {
  tcu.update_acc_voltage(digitalRead(FREQ_ACCU_PIN));
  tcu.update_tsv_voltage(digitalRead(FREQ_TS_PIN));

  if (can_timer.check()) {
    tcu.send_status_message();
  }

  switch (tcu.get_state()) {
  case STANDBY:
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);

    // Another random guessed value tbh
    if (sdc_power.value.in > 500) {
      Serial.println("Entering PRECHARGE");
      epoch = millis();

      tcu.set_state(PRECHARGE);
    }
    break;

  case PRECHARGE:
    // Look for "too fast" or "too slow" precharge, indicates wiring fault
    // [ms]. Set this to something reasonable after collecting normal precharge
    // sequence data
    const float MIN_EXPECTED = 300;

    // [ms]. Set this to something reasonable after collecting normal precharge
    // sequence data
    const float MAX_EXPECTED = 3000;

    // If a precharge is detected faster than this, an error is
    // thrown - assumed wiring fault. This could also arrest oscillating or
    // chattering AIRs, because the TS will retain some amount of precharge.

    // [ms] Precharge amount must be over TARGET_PERCENT for this long before we
    // consider precharge complete
    const uint32_t SETTLING_TIME = 100;

    // The precharge progress is a function of the accumulator voltage in a %ge
    double prechargeProgress =
        100 * tcu.get_tsv_voltage() / tcu.get_acc_voltage();

    Serial.printf("%5lums %4.1f%%  ACV:%5.1fV TSV:%5.1fV", millis() - epoch,
                  prechargeProgress, tcu.get_acc_voltage(),
                  tcu.get_tsv_voltage());

    // Check if precharge complete
    if (prechargeProgress >= tcu.get_target_precharge_percent()) {
      // Precharge too fast
      if (millis() < epoch + MIN_EXPECTED) {
        tcu.set_state(ERROR);
      }

      // Precharge complete
      else if (millis() > (epoch + SETTLING_TIME)) {
        tcu.set_state(TS_ACTIVE);

        Serial.println("Entering TS_ACTIVE");
        epoch = millis();
      }
    }

    // Precharge too slow
    if (millis() > epoch + MAX_EXPECTED) {
      tcu.set_state(ERROR);
    }
    break;

  case TS_ACTIVE:
    // ms. Time to overlap the switching of AIR and Precharge
    const uint32_t T_OVERLAP = 500;

    digitalWrite(SHUTDOWN_CTRL_PIN, HIGH);

    if (millis() > epoch + T_OVERLAP)
      digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    break;

  case ERROR:
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);
    break;
  }
}
