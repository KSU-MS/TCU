#include "main.hpp"
#include "core_pins.h"

void setup() {
  pinMode(FREQ_ACCU_PIN, INPUT);
  pinMode(FREQ_TS_PIN, INPUT);
  pinMode(PWR_OK_PIN, INPUT);

  pinMode(AIR_CTRL_PIN, OUTPUT);
  pinMode(PRECHARGE_CTRL_PIN, OUTPUT);
  digitalWrite(PRECHARGE_CTRL_PIN, LOW);
  digitalWrite(AIR_CTRL_PIN, LOW);

  pinMode(STANDBY_LED, OUTPUT);
  pinMode(PRECHARGE_LED, OUTPUT);
  pinMode(ONLINE_LED, OUTPUT);
  pinMode(ERROR_LED, OUTPUT);
  digitalWrite(STANDBY_LED, HIGH);
  digitalWrite(PRECHARGE_LED, LOW);
  digitalWrite(ONLINE_LED, LOW);
  digitalWrite(ERROR_LED, LOW);

  Serial.begin(9600);
  Serial.println("Entering STANDBY");
}

void loop() {
  tcu.update_acc_voltage(digitalReadFast(FREQ_ACCU_PIN));
  tcu.update_tsv_voltage(digitalReadFast(FREQ_TS_PIN));
  sdc_power.update();

  if (can_timer.check()) {
    tcu.send_status_message();
  }

  switch (tcu.get_state()) {
  case STANDBY:
    digitalWrite(AIR_CTRL_PIN, LOW);
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);

    // Another random guessed value tbh
    if (sdc_power.value.in > 500) {
      Serial.println("Entering PRECHARGE");

      digitalWrite(STANDBY_LED, LOW);
      digitalWrite(PRECHARGE_LED, HIGH);
      digitalWrite(ONLINE_LED, LOW);
      digitalWrite(ERROR_LED, LOW);

      tcu.set_state(PRECHARGE, millis());
      digitalWrite(PRECHARGE_CTRL_PIN, HIGH);
    }
    break;

  case PRECHARGE:
    delay(4000);
    Serial.println("Entering ONLINE");

    digitalWrite(STANDBY_LED, LOW);
    digitalWrite(PRECHARGE_LED, LOW);
    digitalWrite(ONLINE_LED, HIGH);
    digitalWrite(ERROR_LED, HIGH);

    tcu.set_state(TS_ACTIVE, millis());

    // Serial.printf("%5lums %4.1f%%  ACV:%5.1fV TSV:%5.1fV\n",
    //               millis() - tcu.get_state_epoch(),
    //               tcu.get_precharge_percent(), tcu.get_acc_voltage(),
    //               tcu.get_tsv_voltage());
    //
    // // Check if precharge complete
    // if (tcu.get_precharge_percent() >= tcu.get_target_precharge_percent() &&
    //     (millis() > tcu.get_state_epoch() + SETTLING_TIME)) {
    //   // Precharge too fast
    //   if (millis() < tcu.get_state_epoch() + MIN_EXPECTED + SETTLING_TIME) {
    //     tcu.set_state(PRECHARGE_ERROR, millis());
    //
    //     Serial.println("ERROR, TCU PRECHARGED TOO FAST");
    //   }
    //
    //   // Precharge complete
    //   else {
    //     tcu.set_state(TS_ACTIVE, millis());
    //
    //     Serial.println("Entering TS_ACTIVE");
    //   }
    // }
    //
    // // Precharge too slow
    // if (millis() > tcu.get_state_epoch() + MAX_EXPECTED) {
    //   tcu.set_state(PRECHARGE_ERROR, millis());
    //
    //   Serial.println("ERROR, TCU PRECHARGED TOO SLOW");
    // }
    break;

  case TS_ACTIVE:
    digitalWrite(AIR_CTRL_PIN, HIGH);

    if (serial_timer.check()) {
      Serial.printf("acc freq: %f\tacc voltage: %f\n",
                    tcu.acc_opto->get_frequency(), tcu.get_acc_voltage());

      Serial.printf("tsv freq: %f\ttsv voltage: %f\n",
                    tcu.tsv_opto->get_frequency(), tcu.get_tsv_voltage());
    }

    if (millis() > tcu.get_state_epoch() + T_OVERLAP)
      digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    break;

  case PRECHARGE_ERROR:
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    digitalWrite(AIR_CTRL_PIN, LOW);

    digitalWrite(STANDBY_LED, LOW);
    digitalWrite(PRECHARGE_LED, LOW);
    digitalWrite(ONLINE_LED, LOW);
    digitalWrite(ERROR_LED, HIGH);

    if (serial_timer.check()) {
      Serial.printf("acc freq: %f\tacc voltage: %f\n",
                    tcu.acc_opto->get_frequency(), tcu.get_acc_voltage());

      Serial.printf("tsv freq: %f\ttsv voltage: %f\n",
                    tcu.tsv_opto->get_frequency(), tcu.get_tsv_voltage());
    }
    break;
  }
}
