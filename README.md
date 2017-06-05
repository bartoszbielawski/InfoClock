# InfoClock v 0.2.5

A small and configurable clock project that displays some information on a LED display.

## Features
* LED display for displaying information
* Temperature/pressure display using OpenWeatherMap
* Local temperature display
* Embedded web server for configuration and monitoring
* Can log messages using syslog
* LHC Status display 
* Modular approach makes it easy to add new modules (tasks).

## Parts:
* CPU:      ESP8266 (WeMos D1 Mini or NodeMCU)
* Display:  8x8 pixel MAX7219-based display (currently 8 modules)
* DS18B20:  local temperature sensor

## Required libraries:
* ESP8266HTTPClient
* ESP8266WebServer
* ESP8266Wifi
* ESPAsyncTCP (for the LHC Reader)

And libraries already included:
* AJSP (Another JSON Streaming Parser)
* C++Tasks
