/******************************************************************
 RelayControl for Arduino

 Copyright 2025 Brendan Krueger
 Distributed under MIT license - see LICENSE.txt

 See README.md

 File: RelayControl.cpp

******************************************************************/

#include "RelayControl.h"

Relay::Relay(
	const uint8_t relay_pin, uint32_t battery_min_v, uint32_t battery_max_v,
	uint16_t watts_differential
):
	relay_pin(relay_pin)
{
	pinMode(relay_pin, OUTPUT);
	digitalWrite(relay_pin, LOW); 

	if (battery_min_v > -1) {
		bat_min_v = battery_min_v;
	}
	if (battery_max_v > -1) {
		bat_max_v = battery_max_v;
	}
	if (watts_diff > -1) {
		watts_diff = watts_differential;
	}
}

uint8_t Relay::setRelay(uint8_t state)
{
  if (state == 1)
  {
    digitalWrite(relay_pin, HIGH); // Switch to battery
  } 
  else if (state == 0)
  {
    digitalWrite(relay_pin, LOW); // Switch to grid
  }
  return state;
}

/*!
 *  @brief  Sets relay to grid
 *  @return result
 *          returns 0 (relay open).
 */
bool Relay::switchToGrid() {
  digitalWrite(relay_pin, LOW);
  Relay::relay_state = 0;
  return Relay::relay_state; 
}
/*!
 *  @brief  Sets relay to battery
 *  @return result
 *          returns 1 (relay closed).
 */
bool Relay::switchToBattery() {
  digitalWrite(relay_pin, HIGH);
  Relay::relay_state = 1;
  return Relay::relay_state;
}

/*!
 *  @brief  Sets relay if max/min batt voltage exceeded.
 *          Kept as separate function in case PPV not desired.
 *          Not used in setRelay_PrioritizeSolar to avoid extra if statement.
 *  @param  bat_voltage
 *          Current voltage of battery.
 *  @return result
 *          returns 8 bit int representing outcome
 *          0 means relay is open (grid)
 *          1 means relay is closed (battery)
 *          2 means max or min voltage not exceeded.
 */
uint8_t Relay::setRelay_ByBatVoltage(
		uint32_t bat_voltage
	)
{
  if (bat_voltage < bat_min_v) {
	base_state = switchToGrid();
  } else if (bat_voltage > bat_max_v) {
	base_state = switchToBattery();
  } else {
    return 2;
  }
  return base_state;
}

/*!
 *  @brief  Sets relay based on current solar vs load & max/min batt voltages.
 *  @param  watts_solar
 *          Current solar power.
 *  @param  watts_load
 *          Current load power.
 *  @param  bat_voltage
 *          Current voltage of battery.
 *  @return result
 *          returns 8 bit int representing outcome
 *          0 means relay is open (grid)
 *          1 means relay is closed (battery)
 *          2 means load & PV are within differential, so no change.
 */
uint8_t Relay::setRelay_PrioritizeSolar(
	uint16_t watts_solar, uint16_t watts_load,
	uint32_t bat_voltage
  )
{
  uint8_t pv_exceeds_load;
  
  if (bat_voltage < bat_min_v) {
    base_state = switchToGrid();
    return base_state;
  } else if (bat_voltage > bat_max_v) {
    base_state = switchToBattery();
    return base_state;
  } else {
    pv_exceeds_load = pvExceedsLoad(watts_solar, watts_load, watts_diff);
      if (pv_exceeds_load == 1) {
        return switchToBattery();
      } else if (pv_exceeds_load == 0) {
        if (base_state == 1) { // If on battery before solar > load event
          return switchToBattery(); 
          // Still trigger pin in case relay state changed during 
		      // solar > load event due to external issue.
        } else {
          return switchToGrid(); 
        }
      }
    return pv_exceeds_load;
  }
} 

/*!
 *  @brief  Checks if solar power wattage is higher than current load.
 *          includes +/= differential
 *  @param  watts_solar
 *          Current watts produced by solar power.
 *  @param  watts_load
 *          Current watts being consumed by load.
 *  @param  watts_diff
 *          Desired tolerance between solar and load wattage (differential).
 */
uint8_t Relay::pvExceedsLoad(
  uint16_t watts_solar, uint16_t watts_load, uint16_t watts_diff
)
{
  if (watts_solar >= (watts_load + watts_diff)) {
    pv_exceeds_load = 1;
    return 1;
  } else if (watts_solar <= (watts_load - watts_diff)) {
    pv_exceeds_load = 0;
    return 0;
  } else {
    pv_exceeds_load = 2;
    return 2;
  }
}