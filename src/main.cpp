#include "main.hpp"

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

    // TODO: Requires suitable value during commissioning (eg 95%)
    const float TARGET_PERCENT = 70;

    // [ms] Precharge amount must be over TARGET_PERCENT for this long before we
    // consider precharge complete
    const unsigned int SETTLING_TIME = 100;

    static unsigned long epoch;
    static unsigned long tStartPre;

    if (lastState != STATE_PRECHARGE) {
      digitalWrite(PRECHARGE_CTRL_PIN, HIGH);
      lastState = STATE_PRECHARGE;
      statusLEDsOff();
      statusLED[1].on();
      sprintf(lineBuffer, " === PRECHARGE   Target precharge %4.1f%%\n",
              TARGET_PERCENT);
      Serial.print(lineBuffer);
      epoch = now;
      tStartPre = now;
    }

    // Sample the voltages and update moving averages
    const unsigned long samplePeriod = 10; // [ms] Period to measure voltages
    static unsigned long lastSample = 0;

    // samplePeriod and movingAverage alpha value will affect moving average
    // response.
    if (now > lastSample + samplePeriod) {
      lastSample = now;
      ACV_Average.update(getAccuVoltage());
      TSV_Average.update(getTsVoltage());
    }

    // The precharge progress is a function of the accumulator voltage in a %ge
    double prechargeProgress =
        100.0 * TSV_Average.value() / ACV_Average.value();

    // Print Precharging progress
    static unsigned long lastPrint = 0;
    if (now >= lastPrint + 100) {
      lastPrint = now;
      sprintf(lineBuffer, "%5lums %4.1f%%  ACV:%5.1fV TSV:%5.1fV\n",
              now - tStartPre, prechargeProgress, ACV_Average.value(),
              TSV_Average.value());
      Serial.print(lineBuffer);
    }

    // Check if precharge complete
    if (prechargeProgress >= TARGET_PERCENT) {
      // Precharge complete
      if (now > epoch + SETTLING_TIME) {
        state = STATE_ONLINE;
        sprintf(lineBuffer, "* Precharge complete at: %2.0f%%   %5.1fV\n",
                prechargeProgress, TSV_Average.value());
        Serial.print(lineBuffer);
      } else if (now < tStartPre + MIN_EXPECTED &&
                 now > epoch + SETTLING_TIME) { // Precharge too fast -
                                                // something's wrong!
        state = STATE_ERROR;
        errorCode |= ERR_PRECHARGE_TOO_FAST;
      }
    } else {
      // Precharging
      epoch = now;

      if (now >
          tStartPre + MAX_EXPECTED) { // Precharge too slow - something's wrong!
        state = STATE_ERROR;
        errorCode |= ERR_PRECHARGE_TOO_SLOW;
      }
    }
    break;

  case TS_ACTIVE:
    // ms. Time to overlap the switching of AIR and Precharge
    const unsigned int T_OVERLAP = 500;

    static unsigned long epoch;

    if (lastState != STATE_ONLINE) {
      statusLEDsOff();
      statusLED[2].on();
      Serial.println(F(" === RUNNING"));
      lastState = STATE_ONLINE;
      epoch = now;
    }

    digitalWrite(SHUTDOWN_CTRL_PIN, HIGH);
    if (now > epoch + T_OVERLAP)
      digitalWrite(PRECHARGE_CTRL_PIN, LOW);
  }

  void errorState() {
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);

    if (lastState != STATE_ERROR) {
      lastState = STATE_ERROR;
      statusLEDsOff();
      statusLED[3].update(50, 50); // Strobe STS LED
      Serial.println(F(" === ERROR"));

      // Display errors: Serial and Status LEDs
      // TODO use extra bytes in canpacket to send these as well
      if (errorCode == ERR_NONE) {
        Serial.println(F("   *Error state, but no error code logged..."));
      }
      if (errorCode & ERR_PRECHARGE_TOO_FAST) {
        Serial.println(
            F("   *Precharge too fast. Suspect wiring fault / chatter "
              "in shutdown circuit."));
        statusLED[0].on();
      }
      if (errorCode & ERR_PRECHARGE_TOO_SLOW) {
        Serial.println(F("   *Precharge too slow. Potential causes:\n   - "
                         "Wiring fault\n   - "
                         "Discharge is stuck-on\n   - Target precharge percent "
                         "is too high"));
        statusLED[1].on();
      }
      if (errorCode & ERR_STATE_UNDEFINED) {
        Serial.println(F("   *State not defined in The State Machine."));
      }
    }
    delay(500); // WRITE_RESTART(0x5FA0004);
    break;

  case ERROR:
    digitalWrite(PRECHARGE_CTRL_PIN, LOW);
    digitalWrite(SHUTDOWN_CTRL_PIN, LOW);

    if (lastState != STATE_ERROR) {
      lastState = STATE_ERROR;
      statusLEDsOff();
      statusLED[3].update(50, 50); // Strobe STS LED
      Serial.println(F(" === ERROR"));

      // Display errors: Serial and Status LEDs
      // TODO use extra bytes in canpacket to send these as well
      if (errorCode == ERR_NONE) {
        Serial.println(F("   *Error state, but no error code logged..."));
      }
      if (errorCode & ERR_PRECHARGE_TOO_FAST) {
        Serial.println(
            F("   *Precharge too fast. Suspect wiring fault / chatter "
              "in shutdown circuit."));
        statusLED[0].on();
      }
      if (errorCode & ERR_PRECHARGE_TOO_SLOW) {
        Serial.println(F("   *Precharge too slow. Potential causes:\n   - "
                         "Wiring fault\n   - "
                         "Discharge is stuck-on\n   - Target precharge percent "
                         "is too high"));
        statusLED[1].on();
      }
      if (errorCode & ERR_STATE_UNDEFINED) {
        Serial.println(F("   *State not defined in The State Machine."));
      }
    }
    delay(500); // WRITE_RESTART(0x5FA0004);
    break;
  }
}
