#include <Arduino.h>
#include "src/libraries/VictronVEDirectArduino/src/VEDirect.h"
#include "src/libraries/DisplayHandler/src/Displays/display_SH1106/display_SH1106.h"
#include "src/libraries/RelayControl/src/RelayControl.h"

const uint8_t rxPin = 0;
const uint8_t txPin = 1;

// VEDirect initiated with relevant serial object
VEDirect VE_MPPT(Serial1);
uint32_t VED_last_read; // Stores time since last VE.Direct read

/*!
 *  @brief  Represents readings of an energy system.
 *          Values are accessible by variable names and also stored within
 *          the 'readings' array.
 *          This allows different readings to be accessed by their name,
 *          with an additional simple way to iterate through them.
 *          Parallel arrays represent other values tied to the given reading 
 *          at that index.
 */
struct EnergySys {
  #define NUM_READINGS 5
  union {
    struct {
      uint32_t load_watts; // Placeholder value for testing
      uint32_t bat_voltage;
      uint32_t bat_current;
      uint32_t pv_watts;
      uint32_t pv_voltage;
    };
    uint32_t readings[NUM_READINGS];
  };
  // Arrays parallel to 'readings', storing data pertaining to each reading.
  uint32_t last_reads[NUM_READINGS] {0, 0, 0, 0, 0};  // Previous readings
  uint8_t bad_reads[NUM_READINGS] {0, 0, 0, 0, 0};  // Counters for # of contiguous 
                                                    // unsuccessful reads
};

bool check_read_value(uint32_t value, uint32_t last_value, uint16_t diff,
  int8_t bad_read_condition=0);

EnergySys SolarSystem;

uint16_t watts_diff = 5;
// Max and Min battery voltages for low battery switch-to-grid
// in milliVolts
const uint32_t bat_min_v = 12700; // 12.7 volts
const uint32_t bat_max_v = 13600;  // 13.6 volts

const uint8_t relay_pin = 7; // Relay digital output pin
Relay Relay(relay_pin, bat_min_v, bat_min_v, watts_diff);
uint32_t relay_set_interval = 1000;
uint32_t relay_last_set; // time since relay last set 

display_SH1106 OLED_SH1106;
// Int to Character Array Conversions for Display
//////////////////////////////////////////////////
char bat_volts_str[7] = "";
char watts_pv_str[5] = "";
char watts_load_str[5] = "";
char watts_diff_str[4] = "";
char bat_min_v_str[6] = "";
char bat_max_v_str[6] = "";

//////////////////////////////////////////////////////////////////////////////

// The following functions produce str/char arrays to send to visual displays
// Any modules/functions that interacts with a display directly should recieve
// char arrays rather than do translations within the display module.

//////////////////////////////////////////////////////////////////////////////

/*!
 *  @brief  Returns short str representing if relay is on grid or battery.
 *  @param  relay_state
 *          If 0, will return "Grid", if 1, will return "Batt".
 */
char * getRelayStateStr(uint8_t relay_state)
{
  static char output_str[8] = "Error";
  if (relay_state == 0) 
  {
    strcpy(output_str, "Grid");
    return output_str;
  } else if (relay_state == 1) {
    strcpy(output_str, "Batt");
    return output_str;
  } else {
    strcpy(output_str, "ERROR");
    return output_str;
  }
}

/*!
 *  @brief  Returns short str representing last battery voltage trigger reached
 *  @param  base_state
 *          If 0, will return "LoV", if 1, will return "HiV".
 *          LoV = Low Battery Voltage was last triggered.
 *          HiV = High Battery Voltage was last triggered.
 */
char * getLastVTrigger(uint8_t base_state)
{
  static char output_str[4] = "Err";
  if (base_state == 1) 
  {
    strcpy(output_str, "HiV");
    return output_str;
  } else if (base_state == 0){
    strcpy(output_str, "LoV");
    return output_str;
  } else {
    strcpy(output_str, "ERR");
    return output_str;
  }
}

/*!
 *  @brief  Returns string indicating if PPV (Prioritize PV) mode is running.
 *          If PPV mode is not on, returns empty string.
 *  @param  pv_exceeds_load
 *          Represents if solar wattage is greater than load wattage.
 *  @param  relay_state
 *          0 means relay on grid, 1 means relay on battery.
 *  @param  base_state
 *          Hi/Lo voltage last triggered, regardless of actual relay state.
 */
char * getPPV_str(uint8_t pv_exceeds_load, uint8_t relay_state, uint8_t base_state)
{
  static char output_str[6] = "ERROR";
  const char ppv_str[6] = "[PPV]";
  if (
    (base_state == 0) and // Relay is in grid/battery charging mode.
    (
      (pv_exceeds_load == 1) or 
      (
        (pv_exceeds_load == 2) and (relay_state == 1)
      )
    )
  )
  {
    strcpy(output_str, ppv_str);
    return output_str;
  } else {
    strcpy(output_str, "");
    return output_str;
  }
}

/*!
 *  @brief  Returns string of voltage with decimal point.
 *  @param  voltage
 *          Voltage in milliVolts.
 *  @param  output_str
 *          String to be written to.
 */
void getVStr(uint32_t voltage, char output_str[6])
{
  Serial.println(output_str);
  char v_str[7] = "";
  itoa (voltage, v_str, 10);
    Serial.println(v_str);

  if (voltage >= 10000) 
  {
    output_str[0] = v_str[0];
    output_str[1] = v_str[1];
    output_str[2] = '.';
    output_str[3] = v_str[2];
    output_str[4] = v_str[3];
  } else {
    output_str[0] = v_str[0];
    output_str[1] = '.';
      if (voltage <= 10)
    {
      output_str[2] = '0';
    } else 
    {
      output_str[2] = v_str[1];
    }
    output_str[3] = v_str[2];
  }
}

//////////////////////////////////////////////////////////////////////////////

/*!
 *  @brief  Indirect check for bad reads, iterating through all readings, 
 *          keeps previous value unless bad values persist for > tolerance
 *          bad_reads counters only count up until they've exceeded tolerance.
 *  @param  SolarSystem
 *          Struct representing an energy system. Parallel arrays keep track 
 *          of readings, previous readings, and num of consecutive bad reads 
 *          for each reading.
 */
uint8_t handle_bad_reads(EnergySys &SolarSystem)
{
  const uint8_t tolerance = 5;

  for (uint8_t i = 0; i < NUM_READINGS; i++) // Loop thru parallel readings arrays.
  {
    bool read_passed;
    read_passed = check_read_value(SolarSystem.readings[i], SolarSystem.last_reads[i], 5);

    if (read_passed) {
      // Skip to bottom.
      // May add code here if handling gets more complex.
    } else if (SolarSystem.bad_reads[i] < tolerance)
    {
      SolarSystem.bad_reads[i]++;
      // Replace bad read with last good read
      // unless consecutive bad reads reaches tolerance.
      SolarSystem.readings[i] = SolarSystem.last_reads[i];
      continue; 
    }
    // Update lasts if read successful, or bad read tolerance reached.
    SolarSystem.last_reads[i] = SolarSystem.readings[i];
    SolarSystem.bad_reads[i] = 0;
    continue; 
  }
}

bool check_read_value(
  uint32_t value, uint32_t last_value, uint16_t diff, int8_t bad_read_condition
)
{
  // bad_read_condition defined in case what determines a bad read differs
  // per reading.
  // Differential 'diff' assumes readings that were peviously close to 0
  // may reach 0 and still be considered good readings.
  if ((value == bad_read_condition) and (last_value > diff)) {
    return false;
  } 
  return true;
}

//////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(relay_pin, OUTPUT);
  Serial.begin(9600);
  VE_MPPT.begin();
  OLED_SH1106.startScreen();
}

void loop() {
  uint32_t current_millis = millis();

  if (current_millis - VED_last_read > relay_set_interval) 
  {
    SolarSystem.bat_voltage = VE_MPPT.read(VE_BAT_VOLTAGE);
    SolarSystem.pv_watts = VE_MPPT.read(VE_PV_POWER);
    SolarSystem.bat_current = VE_MPPT.read(VE_BAT_CURRENT);

    SolarSystem.load_watts = ( // Convert from milli scale
      (SolarSystem.bat_voltage/1000) * (SolarSystem.bat_current/1000));
    VED_last_read = current_millis;
  }

  if ((current_millis - relay_last_set) > relay_set_interval)
  {
    Relay.setRelay_PrioritizeSolar(
      SolarSystem.pv_watts, SolarSystem.load_watts, SolarSystem.bat_voltage);
    getVStr(SolarSystem.bat_voltage, bat_volts_str);
  
    OLED_SH1106.printAllReadouts(
      (itoa (SolarSystem.pv_watts, watts_pv_str, 10)),
      (itoa (SolarSystem.load_watts, watts_load_str, 10)),
      bat_volts_str,
      getRelayStateStr(Relay.relay_state),
      getLastVTrigger(Relay.base_state), 
      getPPV_str(Relay.pv_exceeds_load, Relay.relay_state, Relay.base_state)
      );
    relay_last_set = current_millis;
  }
}