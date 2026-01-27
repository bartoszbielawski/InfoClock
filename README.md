# InfoClock v 0.5.7

A small and configurable clock project that displays some information on a LED display.

Configured for use with PlatformIO

## Features
* LED display for displaying information
* Temperature and forecast display using OpenWeatherMap
* Local temperature display (DS18B20)
* Embedded web server for configuration and monitoring
* Can log messages using syslog
* LHC Status display (using data from ALICE) (currently disabled, the LHC is off for maintenance)
* Currency exchange rate from http://fixer.io (disabled - broken due to API changes)
* Modular approach makes it easy to add new modules (tasks)
* Stateless messages, configurable with end/start date, displayable with countdown/count-up option.
* Temperature offset
* large alphabet
* 5 languages (english, french, spanish, portugese, german)

## Parts:
* CPU:      ESP8266 (WeMos D1 Mini or NodeMCU)
* Display:  8x8 pixel MAX7219-based display (currently 8 modules)
* DS18B20:  local temperature sensor

## Required libraries:

### Global:
* ESP8266HTTPClient
* ESP8266WebServer
* ESP8266Wifi
* Adarfuit GFX

### From Arduino library repo
* AJSP (https://github.com/bartoszbielawski/InfoClock)
* CPPTasks (https://github.com/bartoszbielawski/CPPTasks)
* LEDMatrixDisplay (https://github.com/bartoszbielawski/LEDMatrixDriver)

### Contributors
* Arkadiusz Gorzawski (https://github.com/agorzawski)
