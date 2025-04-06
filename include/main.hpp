#pragma once

#include <Arduino.h>
#include <Metro.h>

#include <adc.hpp>

#include "freq.hpp"
#include "pin_defs.hpp"
#include "tcu.hpp"

adc sdc_power(avr, PWR_OK_PIN);

freq acc_opto;
freq tsv_opto;

canMan acc_can(TEENSY_CAN1, ACCUMULATOR_CAN_BAUD_RATE);
can_obj_car_h_t dbc;

// This 90 is 90% for the precharge target, ngl picked this at random...
TCU tcu(90, &acc_opto, &tsv_opto, &acc_can, &dbc);

Metro can_timer(1000);

uint32_t epoch;
