/*
 * OTA.cpp

 *
 *  Created on: 17.04.2017
 *      Author: caladan
 */

#include "Arduino.h"
#include "ArduinoOTA.h"
#include "pgmspace.h"
#include "OTA.hpp"
#include "utils.h"
#include "ESP8266mDNS.h"

void configureOTA()
{
	ArduinoOTA.onStart([]() {
		logPrintf(F("Starting OTA..."));
	});

	ArduinoOTA.onEnd([]() {
		logPrintf(F("OTA done..."));
	});

	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		logPrintf(F("Progress: %u%%"), (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		logPrintf(F("Error[%u]"), error);
	});

	ArduinoOTA.setPassword(readConfig(F("configPassword")).c_str());

	ArduinoOTA.begin();
}


