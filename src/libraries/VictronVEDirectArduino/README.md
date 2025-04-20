# VictronVEDirectArduino
Lightweight Arduino library to read data using the VE.Direct protocol from Victron Energy devices

Built as part of a larger project, now making it available separately in case others find it useful.

Setup:
 - An Arduino(ish) board 
 - A Victron Energy device that sends serial data using the text version of the VE.Direct protocol
 - A 5v to 3.3v serial converter (BMV is 3.3v - don't plug it directly into a 5v Arduino!)
 - Plugged into the Arduino on a serial port (eg Serial1, Serial2, Serial3 etc)
 - See also: https://www.victronenergy.com/live/vedirect_protocol:faq
 - Developed and tested with a BMV-700 battery monitor, should work with any that talk VE.Direct
 - Distributed under an MIT license - see LICENCE.txt

Provides:
 - Read access to VE.Direct statistics and settings
 - Defaults set to read Volts, Power, Current, State of Charge (SOC), Alarm
 - Easily extendible by adding labels for any other stats and settings of interest
 - A diagnostic "full dump" of everything coming from the device  

### Usage:
```C
#include "VEDirect.h"

VEDirect my_bmv(Serial3);
my_int32 = my_bmv.read(VE_SOC);	

// VE_SOC, VE_VOLTAGE, VE_CURRENT, VE_POWER, VE_ALARM
```
### Changes to Original (by Brendan Krueger):
Changes in this fork include:
- Added a timeout within the read() function that prevents the library from hanging indefinitely if the VE.Direct cable is physically disconnected. The length of the timeout can be set with the serial_timout variable.
- Added labels for other retrievable fields, though just to be safe, only those that are readable on the victron hardware that I have - a MPPT 100/50 bought in 2024 - which should work for other MPPT units from around this time.
- Renamed some labels to better differentiate between battery and solar pv fields.
- VEDirect.read() no longer includes a call to VESerial.begin(). This was found unecessary in my testing.
- VED_MAX_LEBEL_SIZE macro renamed to VED_MAX_LABEL_SIZE, as I believe the original was a misspelling.

- Different Victron products produce different fields/labels depending on the type of hardware and what data it has access to. It is best to only run VEDirect.read() to find data you know your specific hardware produces or else, in my experience, this library will spend a noticeable amount of time searching for it unecessarily.