#pragma once

#define ACCUMULATOR_CAN_BAUD_RATE 500000

// Configuration solder-jumpers
#define CONFIG_PIN [] { 4, 5, 6 }
// Status LEDs
#define STATUS_LED [] { 0, 1, 2, 3 }

// Relays
#define SHUTDOWN_CTRL_PIN 13
#define PRECHARGE_CTRL_PIN 19

// Frequency measurements (from Voltage-to-Frequency converters)
#define FREQ_ACCU_PIN 14
#define FREQ_TS_PIN 15

// Active-high when power-supply (shutdown circuit) is OK
#define PWR_OK_PIN 18 // A4

// Accumulator V-F converter - Schematic REF : U8
#define V2F_slope_accu 7.19  // Gradient constant
#define V2F_ofs_accu -6.2818 // offset / y-intercept consant

// Tractive System V-F converter - Schematic REF: U7
#define V2F_slope_ts 7.22  // Gradient constant
#define V2F_ofs_ts -8.7202 // offset / y-intercept consant

// Voltage dividers upstream of V-F converters:
// R31+R32+R36+R37 = 2.55M, R39 = 39k, Rtotal = 2.55M+39k
// gain = R39/(Rtotal)
// Our values:
// 66500/(2550000+66500)
#define gainVoltageDivider 0.02541563156
