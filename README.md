# SolarController (alpha)

This is a simpler demonstration version of a larger project designed around my most recent small-scale solar system.

## Features:
### Dynamic Battery-to-Grid Relay Control
Control a relay to dynamically switch between grid power and battery power based on current state of solar system. The `Relay` class represents a relay controlled by a selected Arduino pin.
- `Relay.setRelay_ByBatVoltage`: For simple switching based on battery charge state, this method switches relay to battery when a set battery high voltage is reached, and to grid power when a set low voltage is reached, remaining on grid power until the battery is charged to the given high voltage.
- `Relay.setRelay_PrioritizeSolar`: This method dynamically switches to solar power whenever the amount of solar energy generated exceeds the load. It follows the high and low battery voltage settings, but when the system is in 'charging mode' (Low voltage was last voltage trigger), it will switch back to battery as long as incoming PV power is greater than the load, then continue charging the battery when the incoming power drops below the load. This allows generated power to be used directly more often, theoretically increasing the efficiency of the system. A differential prevents the relay from switching too rapidly while PV and load wattage are nearly equal.

### Communicate with Victron Energy devices via the [VE.Direct protocol](https://www.victronenergy.com/upload/documents/Technical-Information-Data-communication-with-Victron-Energy-products_EN.pdf).  
- If using VE.Direct compatible Victron products such as MPPT charge controllers or battery monitors, A serial connection allows this module to parse available data, such as current solar & load wattage and battery voltage, to be used by the Relay class's algorithms.
- This module includes a modified version of the [VictronVEDirectArduino](https://github.com/winginitau/VictronVEDirectArduino) library. I added a timeout to the `read()` function so if the serial cable is physically disconnected or the connection is otherwise lost, the program will continue running.
- The `handle_bad_reads` function may be used to ignore stray readings for a set number of loops. This can prevent the relay from switching unnecessarily.
  
### Show System Status and Parameters with external displays.
- Compatible with SH1106 type OLED Displays. (Requires the `ss_oled` library)
- Display current solar power, load, and battery voltage.
- Display status of controlled relay, including on/off state, last voltage trigger, and 'Prioritize PV' status.

## In Progress
Various additions are in progress or being tested for the 'full' version, including:
- Visualizing system status with the Arduino Uno R4 Wifi's built in LED matrix.
- A more modular `DisplayHandler` library to more intuitively support adding a wider array of external displays.
- Better integration for multiple VE.Direct connections.
- Improvements to the relay switching algorithm, allowing more points of feedback from the solar system.

## Setup and Compatibility:
The included `SolarController_alpha.ino` file is a simple example project. Exact usage and methods for obtaining system state variables may need to be changed depending on the particular equipment used.

This module is currently being used and tested with the following:
 - An Arduino Uno R4 Wifi board.
 - Victron MPPT controllers.
 - Load measured via a shunt and an INA169 gain current sense amplifier.
