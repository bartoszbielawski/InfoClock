# InfoClock v 0.3.0

A small and configurable clock project that displays some information on a LED display.

## Features
* LED display for displaying information
* Temperature and forecast display using OpenWeatherMap
* Local temperature display
* Embedded web server for configuration and monitoring
* Can log messages using syslog
* LHC Status display (using data from ALICE)
* Modular approach makes it easy to add new modules (tasks).

## Parts:
* CPU:      ESP8266 (WeMos D1 Mini or NodeMCU)
* Display:  8x8 pixel MAX7219-based display (currently 8 modules)
* DS18B20:  local temperature sensor

## Required libraries:

### Global:
* ESP8266HTTPClient
* ESP8266WebServer
* ESP8266Wifi

### From Arduino library repo
* AJSP (https://github.com/bartoszbielawski/InfoClock)
* CPPTasks (https://github.com/bartoszbielawski/CPPTasks)
* LEDMatrixDisplay (https://github.com/bartoszbielawski/LEDMatrixDriver)
