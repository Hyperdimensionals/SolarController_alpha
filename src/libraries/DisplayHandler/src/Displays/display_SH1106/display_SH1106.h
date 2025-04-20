/*
  display_SH1106.h - Library for displaying environmental parameters using the SH1106 OLED Display.
  Created by Brendan Blaise Krueger 2025.

*/
#ifndef display_SH1106_h
#define display_SH1106_h

#include "Arduino.h"
#include <ss_oled.h>

class display_SH1106
	{
	public:
		SSOLED ssoled;

		void startScreen();

		void printEnvParam(char *param_str, char unit[3], uint8_t x, uint8_t y);
		void printEnvParam_8x8(uint8_t x, uint8_t y,
			char *param_str, char unit[3], char prefix[]);
		void printEnvParam_6x8(
				uint8_t x, uint8_t y,
				char *param_str, char unit[3], char prefix[]);
		void printEnvParamSmall(float param_num, char prefix[8],
                                uint8_t x, uint8_t y);
		void print2Digit(float param_num, uint8_t x, uint8_t y, bool one_digit);

		void printAllReadouts(
			char *pv_watts, char*load_watts, char *bat_volts, char *relay_state, 
			char *last_v_trigger, char *solar_exceeds_load
		);
	
	private:
	};

#endif
