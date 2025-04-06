#pragma once

#include "can_tools.hpp"
#include "car.h"
#include "freq.hpp"
#include "pin_defs.hpp"

enum state {
  STANDBY = 0,
  PRECHARGE = 1,
  TS_ACTIVE = 2,
  ERROR = 3,
};

class TCU {
private:
  state current_state;
  float current_acc_voltage;
  float current_tsv_voltage;
  float target_precharge_percent;

  freq *acc_opto;
  freq *tsv_opto;

  canMan *acc_can;
  can_obj_car_h_t *dbc;

public:
  TCU(float target_precharge_percent, freq *acc_opto, freq *tsv_opto,
      canMan *acc_can, can_obj_car_h_t *dbc) {
    this->target_precharge_percent = target_precharge_percent;
    this->acc_opto = acc_opto;
    this->tsv_opto = tsv_opto;

    this->acc_can = acc_can;
    this->dbc = dbc;

    current_state = STANDBY;
  }

  // Idk this shit really ain't complicated enough for the full state machine
  inline bool set_state(state target_state) { current_state = target_state; }
  inline state get_state() { return current_state; }

  inline double get_target_precharge_percent() {
    return this->target_precharge_percent;
  }

  float get_acc_voltage() {
    this->current_acc_voltage = (acc_opto->get_frequency() - V2F_ofs_accu) /
                                V2F_slope_accu / gainVoltageDivider;
    return this->current_acc_voltage;
  }
  float get_tsv_voltage() {
    this->current_tsv_voltage = (tsv_opto->get_frequency() - V2F_ofs_ts) /
                                V2F_slope_ts / gainVoltageDivider;
    return this->current_tsv_voltage;
  }

  inline void update_acc_voltage(bool pin_state) {
    acc_opto->update(pin_state);
  }
  inline void update_tsv_voltage(bool pin_state) {
    tsv_opto->update(pin_state);
  }

  void send_status_message() {
    encode_can_0x069_precharge_accVoltageDiv100(dbc, current_acc_voltage / 100);
    // encode_can_0x069_precharge_accVoltageMod100(dbc, uint8_t in);
    encode_can_0x069_precharge_tsVoltageDiv100(dbc, current_tsv_voltage / 100);
    // encode_can_0x069_precharge_tsVoltageMod100(dbc, current_tsv_voltage %
    // 100);
    encode_can_0x069_precharge_errorCode(dbc, 0);
    encode_can_0x069_precharge_state(dbc, current_state);

    can_message out;
    pack_message(dbc, CAN_ID_PRECHARGE_STATUS, &out.buf.val);
    acc_can->send_controller_message(out);
  }
};
