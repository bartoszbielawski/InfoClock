/*
 * LocalSensorTask.cpp
 *
 *  Created on: 04.04.2017
 *      Author: caladan
 */

#include "LocalSensorTask.h"
#include "DataStore.h"
#include "utils.h"
#include "tasks_utils.h"
#include "config.h"
#include "web_utils.h"
#include "WebServerTask.h"
#include <DisplayTask.hpp>

LocalSensorTask::LocalSensorTask():
	oneWire(ONE_WIRE_TEMP),
	dallasTemperature(&oneWire)
{
	//this is need if there are no free GND pins
#ifdef OW_GND
	pinMode(OW_GND, OUTPUT);
	digitalWrite(OW_GND, 0);
#endif

	//initialize and request the temperature right away
	dallasTemperature.begin();
	dallasTemperature.setWaitForConversion(false);
	dallasTemperature.requestTemperatures();

	registerPage(F("lst"), F("Local Sensors"), [this](ESP8266WebServer& webServer) {handlePage(webServer);});

	addRegularMessage({this, [this](){return formatTemperature();}, 3_s, 1, false});

	sleep(10_s);
}

bool isTemperatureValid(float f)
{
	return (f != -127.0f) && (f != 85.0);
}

void LocalSensorTask::run()
{
	float t = dallasTemperature.getTempCByIndex(0) + readConfig(F("tempOffset")).toInt();
	bool isValid = isTemperatureValid(t);

	if (isValid)
	{
		logPrintfX(F("LST"), F("T = %s deg C"), String(t, 1).c_str());
	}
	else
	{
		logPrintfX(F("LST"), F("Sensor not found..."));
	}

	temperature = t;

	if (DataStore::value(F("lstMqtt")).toInt())
	{
		if (isValid)
			DataStore::value(F("lstTemperature")) = String(t, 1);
		else
			DataStore::erase(F("lstTemperature"));
	}

	dallasTemperature.requestTemperatures();
	sleep(10_s);
}


static const char lstStatusPage[] PROGMEM = R"_(
<table>
<tr><th>Local Sensor Task</th></tr>
<tr><td class="l">Temperature:</td><td>$t$ &#8451;</td></tr>
</table></form></body>
<script>setTimeout(function(){window.location.reload(1);}, 15000);</script>
</html>
)_";

FlashStream lstStatusPageFS(lstStatusPage);

void LocalSensorTask::handlePage(ESP8266WebServer& webServer)
{
	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString(F("LST Status")), ss);

	String tempString(F("Sensor missing"));
	if (isTemperatureValid(temperature))
		tempString = String(temperature);

	macroStringReplace(lstStatusPageFS, constString(String(tempString)), ss);
	webServer.send(200, textHtml, ss.buffer);
}

String LocalSensorTask::formatTemperature()
{
	if (not isTemperatureValid(temperature))
		return F("No sensor!");

	String p = "\x81 ";
	p += String(temperature, 1);
	p += (char)0x80;
	p += "C";
	return p;
}
