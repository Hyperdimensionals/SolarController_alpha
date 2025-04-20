/******************************************************************
 RelayControl for Arduino

 Copyright 2025 Brendan Krueger
 Distributed under MIT license - see LICENSE.txt

 See README.md

 File: RelayControl.h

******************************************************************/

#ifndef RELAYCONTROL_H
#define RELAYCONTROL_H

#include <Arduino.h>

class Relay {
public:
	Relay(const uint8_t relay_pin, uint32_t battery_min_v = -1,
		uint32_t battery_max_v = -1, uint16_t watts_differential = -1);
		// -1 indicates value is not set
	uint8_t relay_pin;
	uint8_t relay_state; // Current actual relay state
	uint8_t base_state; // Stores value of last voltage hi/lo trigger
	uint8_t pv_exceeds_load;
	// Control Settings
	uint32_t bat_min_v = 12700;
	uint32_t bat_max_v = 13600;
	uint16_t watts_diff = 5;

	uint8_t setRelay(uint8_t state);
	bool switchToGrid();
	bool switchToBattery();
	uint8_t setRelay_ByBatVoltage(
		uint32_t bat_voltage
	);
	uint8_t setRelay_PrioritizeSolar(
		uint16_t watts_solar, uint16_t watts_load,
		uint32_t bat_voltage
	);
	uint8_t pvExceedsLoad(
		uint16_t watts_solar, uint16_t watts_load, uint16_t watts_diff
	  );
private:

};

#endif /* RELAYCONTROL_H */
