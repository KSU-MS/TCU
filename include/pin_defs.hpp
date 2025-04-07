#pragma once

#define ACCUMULATOR_CAN_BAUD_RATE 500000

// Configuration solder-jumpers
// #define CONFIG_PIN [] { 4, 5, 6 }

// Status LEDs
// #define STATUS_LED [] { 0, 1, 2, 3 }

// Relays
#define SHUTDOWN_CTRL_PIN 13
#define PRECHARGE_CTRL_PIN 19

// Frequency measurements (from Voltage-to-Frequency converters)
#define FREQ_ACCU_PIN 14
#define FREQ_TS_PIN 15

// Active-high when power-supply (shutdown circuit) is OK
#define PWR_OK_PIN 18 // A4

// Accumulator V-F converter - Schematic REF : U8
#define V2F_slope_accu 7.22  // Gradient constant
#define V2F_ofs_accu -8.7202 // offset / y-intercept consant

// Tractive System V-F converter - Schematic REF: U7
#define V2F_slope_ts 7.22  // Gradient constant
#define V2F_ofs_ts -8.7202 // offset / y-intercept consant

// Voltage dividers upstream of V-F converters: 39k/(2.55M+39k)
#define gainVoltageDivider 0.0150637311703

#define V2F_slope 7
#define V2F_factor (V2F_slope / gainVoltageDivider)

// Look for "too fast" or "too slow" precharge, indicates wiring fault
// [ms]. Set this to something reasonable after collecting normal precharge
// sequence data
#define MIN_EXPECTED 300
// If a precharge is detected faster than this, an error is
// thrown - assumed wiring fault. This could also arrest oscillating or
// chattering AIRs, because the TS will retain some amount of precharge.

// [ms]. Set this to something reasonable after collecting normal precharge
// sequence data
#define MAX_EXPECTED 3000

// [ms] Precharge amount must be over TARGET_PERCENT for this long before we
// consider precharge complete
#define SETTLING_TIME 1000

// ms. Time to overlap the switching of AIR and Precharge
#define T_OVERLAP 500
