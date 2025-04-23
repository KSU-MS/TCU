#pragma once

#include "can_tools.hpp"
#include "car.h"
#include "freq.hpp"
#include "pin_defs.hpp"

enum state {
  STANDBY = 0,
  PRECHARGE = 1,
  TS_ACTIVE = 2,
  PRECHARGE_ERROR = 3,
};

class TCU {
private:
  state current_state;
  uint8_t error_code;
  uint32_t state_epoch;

  float current_acc_voltage;
  float current_tsv_voltage;

  float current_precharge_percent;
  float target_precharge_percent;

  canMan *acc_can;
  can_obj_car_h_t *dbc;

public:
  freq *acc_opto;
  freq *tsv_opto;

  TCU(float target_precharge_percent, freq *acc_opto, freq *tsv_opto,
      canMan *acc_can, can_obj_car_h_t *dbc) {
    this->target_precharge_percent = target_precharge_percent;
    this->acc_opto = acc_opto;
    this->tsv_opto = tsv_opto;

    this->acc_can = acc_can;
    this->dbc = dbc;

    current_state = STANDBY;
    state_epoch = 0;

    current_acc_voltage = 0;
    current_tsv_voltage = 0;
    current_precharge_percent = 0;
  }

  // Idk this shit really ain't complicated enough for the full state machine
  void set_state(state target_state, uint32_t time) {
    current_state = target_state;
    state_epoch = time;
  }
  inline state get_state() { return current_state; }
  inline uint32_t get_state_epoch() { return state_epoch; }

  inline double get_target_precharge_percent() {
    return this->target_precharge_percent;
  }

  inline float get_acc_voltage() { return current_acc_voltage; }
  inline float get_tsv_voltage() { return current_tsv_voltage; }
  inline float get_precharge_percent() {
    return (current_tsv_voltage / current_acc_voltage) * 100;
  }

  void update_acc_voltage(bool pin_state) {
    acc_opto->update(pin_state);
    current_acc_voltage = (acc_opto->get_frequency() - V2F_offset) / V2F_factor;
  }
  void update_tsv_voltage(bool pin_state) {
    tsv_opto->update(pin_state);
    current_tsv_voltage = (tsv_opto->get_frequency() - V2F_offset) / V2F_factor;
  }

  void send_status_message() {
    encode_can_0x069_precharge_state(dbc, current_state);
    encode_can_0x069_precharge_errorCode(dbc, error_code);
    encode_can_0x069_precharge_accVoltageDiv100(dbc, current_acc_voltage / 100);
    encode_can_0x069_precharge_accVoltageMod100(
        dbc, (int(current_acc_voltage) % 100));
    encode_can_0x069_precharge_tsVoltageDiv100(dbc, current_tsv_voltage / 100);
    encode_can_0x069_precharge_tsVoltageMod100(
        dbc, (int(current_tsv_voltage) % 100));

    can_message out;
    out.id = CAN_ID_PRECHARGE_STATUS;
    out.length = pack_message(dbc, CAN_ID_PRECHARGE_STATUS, &out.buf.val);

    acc_can->send_controller_message(out);
  }
};
