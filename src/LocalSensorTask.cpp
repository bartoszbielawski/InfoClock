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
#include <ESP8266WebServer.h>

static const char lstStatusPage[] PROGMEM = R"_(
<table>
<tr><th>Local Sensor Task</th></tr>
<tr><td class="l">Temperature:</td><td>$t$ &#8451;</td></tr>
</table></form></body>
<script>setTimeout(function(){window.location.reload(1);}, 15000);</script>
</html>
)_";

FlashStream lstStatusPageFS(lstStatusPage);

LocalSensorTask::LocalSensorTask():
	oneWire(ONE_WIRE_TEMP),
	dallasTemperature(&oneWire)
{
	//this is need
#ifdef OW_GND
	pinMode(OW_GND, OUTPUT);
	digitalWrite(OW_GND, 0);
#endif

	//initialize and request the temperature right away
	dallasTemperature.begin();
	dallasTemperature.setWaitForConversion(false);
	dallasTemperature.requestTemperatures();
	

	getWebServerTask().registerPage(F("lst"), "Local Sensors", 
		[this](ESP8266WebServer& webServer) {handlePage(webServer);}
	);

	getDisplayTask().addRegularMessage({this, [this](){return formatTemperature();}, 3_s, 1, false});

	sleep(10_s);
}

void LocalSensorTask::run()
{
	float t = dallasTemperature.getTempCByIndex(0);
	if (t == -127.0f)
	{
		logPrintfX(F("LST"), F("Sensor not found..."));
	}
	else
	{	
		logPrintfX(F("LST"), F("T = %s deg C"), String(t, 1).c_str());
	}

	temperature = t;

	if (DataStore::value("lstMqtt").toInt())
	{
		DataStore::value("lstTemperature") = String(t, 1);
	}
	dallasTemperature.requestTemperatures();
	sleep(10_s);
}

void LocalSensorTask::handlePage(ESP8266WebServer& webServer)
{
	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString(F("LST Status")), ss);

	String tempString(F("Sensor missing"));
	if (temperature > -127.0f)
		tempString = String(temperature);

	macroStringReplace(lstStatusPageFS, constString(String(tempString)), ss);
	webServer.send(200, textHtml, ss.buffer);
}

String LocalSensorTask::formatTemperature()
{
	if (temperature == -127.0f)	
		return "No sensor detected!";

	String p = "\x81 ";
	p += String(temperature, 1);
	p += (char)0x80;
	p += "C";
	return p;
}

static RegisterTask lstr(new LocalSensorTask, 0);
