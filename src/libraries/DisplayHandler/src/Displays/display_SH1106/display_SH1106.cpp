/*
  displayOLED_SH1106.cpp - Library for displaying environmental parameters using the SH1106 OLED Display.

*/

#include "Arduino.h"
//#include "U8x8lib.h"
#include "display_SH1106.h"

// if your system doesn't have enough RAM for a back buffer, comment out
// this line (e.g. ATtiny85)
// #define USE_BACKBUFFER

//* SCREEN DEFINING CONSTANTS */
// Use -1 for the Wire library default pins
// or specify the pin numbers to use with the Wire library or bit banging on any GPIO pins
// These are the pin numbers for the M5Stack Atom default I2C
#define SDA_PIN -1
#define SCL_PIN -1
// Set this to -1 to disable or the GPIO pin number connected to the reset
// line of your display if it requires an external reset
#define RESET_PIN -1
#define OLED_ADDR 0x3c // Set to -1 to let ss_oled figure out display address
// don't rotate the display
#define FLIP180 1
// don't invert the display
#define INVERT 0
// Bit-Bang the I2C bus
#define USE_HW_I2C 1

// Change these if you're using a different OLED display
#define OLED_USED OLED_128x64
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

const uint8_t CLOCK_PIN = A4;
const uint8_t DATA_PIN = A5;

void display_SH1106::startScreen() {
  int rc;
  // The I2C SDA/SCL pins set to -1 means to use the default Wire library
  // If pins were specified, they would be bit-banged in software
  // This isn't inferior to hw I2C and in fact allows you to go faster on certain CPUs
  // The reset pin is optional and I've only seen it needed on larger OLEDs (2.4")
  //    that can be configured as either SPI or I2C
  //
  // oledInit(SSOLED *, type, oled_addr, rotate180, invert, bWire, SDA_PIN, SCL_PIN, RESET_PIN, speed)

  rc = oledInit(&ssoled, OLED_USED, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L); // use standard I2C bus at 400Khz
  if (rc != OLED_NOT_FOUND)
  {
    Serial.println("OLED Screen Found");
  }
}
/*!
 *  @brief  Prints an environmental reading with prefix on SSH1106
 *  @param  param_str
 *          Prefix to display (type of reading) (char *)
 *  @param  unit
 *          1-2 character suffix to display (char)
 *  @param  x
 *          X print location on OLED screen (uint8_t)
 *  @param  y
 *          Y print location on OLED screen (uint8_t)
 */
void display_SH1106::printEnvParam(char *param_str, char unit[3], uint8_t x, uint8_t y) 
{
    oledWriteString(&ssoled, 0,x,y, param_str, FONT_12x16,0,1);
    oledWriteString(&ssoled, 0,(x + 50),y, unit, FONT_12x16,0,1);
}

/*!
 *  @brief  Prints parameter small with prefix
 *  @param  param_num
 *          target temperature of controller (float), max 5 characters
 *  @param  prefix
 *          str to preced parameter, max 6 characters
 *  @param  x
 *          X print location on OLED screen (uint8_t)
 *  @param  y
 *          Y print location on OLED screen (uint8_t)
 */
void display_SH1106::printEnvParamSmall(float param_num, char prefix[7],
                                        uint8_t x, uint8_t y)
{
  char param_num_str[6] = "0";
  itoa (param_num, param_num_str, 10);
  oledWriteString(&ssoled, 0,x,y, prefix, FONT_6x8,0,1);
  oledWriteString(&ssoled, 0,(x + 35),y, param_num_str, FONT_6x8,0,1);
}

void display_SH1106::printEnvParam_8x8(
  uint8_t x, uint8_t y,
  char *param_str, char unit[3], char prefix[]) 
{
  oledWriteString(&ssoled, 0,x,y, prefix, FONT_8x8,0,1);
  x += (strlen(prefix)*8 + 2);
  oledWriteString(&ssoled, 0,x,y, param_str, FONT_8x8,0,1);
  x += (strlen(param_str)*8 + 2);
  oledWriteString(&ssoled, 0,x,y, unit, FONT_8x8,0,1);
}

void display_SH1106::printEnvParam_6x8(
  uint8_t x, uint8_t y,
  char *param_str, char unit[3], char prefix[]) 
{
  oledWriteString(&ssoled, 0,x,y, prefix, FONT_6x8,0,1);
  x += (strlen(prefix)*6 + 2);
  oledWriteString(&ssoled, 0,x,y, param_str, FONT_6x8,0,1);
  x += (strlen(param_str)*6 + 2);
  oledWriteString(&ssoled, 0,x,y, unit, FONT_6x8,0,1);
}

/*!
 *  @brief  Prints entire screen readouts
 *  @param  temp
 *          temperature (char *)
 *  @param  temp_status
 *          on/off status of temperature control device (bool)
 *  @param  hum
 *          humidity (char *)
 *  @param  hum_status
 *          on/off status of humidity control device (bool)
 *  @param  target
 *          target temperature of controller (float)
 *  @param  minutes_on
 *          num minutes device has been on (float)
 *  @param  resets
 *          Number of resets for DHT hard reset library
 */
void display_SH1106::printAllReadouts(
  char *pv_watts, char*load_watts, char *bat_volts, char *relay_state, 
  char *last_v_trigger, char *solar_exceeds_load
)
{
  uint8_t x = 2;
  uint8_t y = 0;

  oledFill(&ssoled, 0x0, 1);
  // printBattVoltage(bat_volts, bat_volts);

  printEnvParam_6x8(x, 1, pv_watts, "W", "PV:");
  printEnvParam_6x8(x, 3, bat_volts, "V", "Battery:");
  
  printEnvParam_6x8(x, 5, relay_state, "", "Relay: On");
  printEnvParam_6x8(x, 7, last_v_trigger, "", "Last Trigger:");

  // 2nd Column
  x = 64;
  printEnvParam_6x8(x, 1, load_watts, "W", "Load:");

  printEnvParam_6x8((x + 25), 5, solar_exceeds_load, "", "");

}