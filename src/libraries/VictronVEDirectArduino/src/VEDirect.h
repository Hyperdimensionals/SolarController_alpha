/******************************************************************
 VEDirect Arduino

 Copyright 2018, 2019, Brendan McLearie
 Distributed under MIT license - see LICENSE.txt

 See README.md

 File: VEDirect.h
 - Class / enums / API

 Updates:
 [Changes by Brendan Krueger]
 - 2025-04-20:
	  - Added timeout within read(), so library won't hang if VE.Direct device
	    is disconnected
  	  - Fixed apparent mispelling of VED_MAX_LABEL_SIZE
	  - Added labels for other retrievable fields
	  - Renamed some labels to better differentiate between battery and solar pv fields.
	  - VEDirect.read() no longer includes a call to VESerial.begin()
******************************************************************/

#ifndef VEDIRECT_H_
#define VEDIRECT_H_

#include <Arduino.h>

// Tunable parameters - defaults tested on mega2560 R3
#define VED_LINE_SIZE 30		 // Seems to be plenty. VE.Direct protocol could change
#define VED_MAX_LABEL_SIZE 6	 // Max length of all labels of interest + '\0'. See ved_labels[]
#define VED_MAX_READ_LOOPS 60000 // How many read loops to be considered a read time-out
#define VED_MAX_READ_LINES 50	 // How many lines to read looking for a value
								 // before giving up. Also determines lines for diag dump
#define VED_BAUD_RATE 19200

// Extend this and ved_labels[] for needed inclusions
enum VE_DIRECT_DATA {
	VE_DUMP = 0,
	VE_SOC,
	VE_BAT_VOLTAGE,
	VE_BAT_POWER,
	VE_PV_POWER,
	VE_BAT_CURRENT,
	VE_ALARM,
	VE_LAST_LABEL,
};

const char ved_labels[VE_LAST_LABEL][VED_MAX_LABEL_SIZE] PROGMEM = {
		"Dump",	// a string that won't match any label
		"SOC",
		"V",
		"P",
		"PPV",
		"I",
		"Alarm",
};

class VEDirect {
public:
	VEDirect(HardwareSerial& port, uint32_t serial_timout = 5000);
	virtual ~VEDirect();
	uint8_t begin();
	int32_t read(uint8_t target);
	HardwareSerial& VESerial;
	uint32_t serial_timout;

private:
	uint32_t serial_last_available;
	//HardwareSerial& VESerial;
};

#endif /* VEDIRECT_H_ */
