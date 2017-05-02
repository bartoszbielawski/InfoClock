# InfoClock

A small clock project that displays some information on a LED display

CPU:      ESP8266 (WeMos D1 Mini)

Display:  8x 8x8 pixel MAX7219-based display
Bought here:
https://www.aliexpress.com/item/Free-Shipping-MAX7219-Dot-Matrix-Module-For-Arduino-Microcontroller-4-In-One-Display-with-5P-Line/32597603008.html

The current setup is 2 panels of 2x4 displays. Number of displays can be set in config.h


Required libraries:
* ESP8266HTTPClient
* ESP8266WebServer
* ESP8266Wifi
* LedControl

And libraries already included:
* AJSP (Another Json Streaming Parser)
* C++Tasks
