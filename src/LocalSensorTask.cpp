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
	dallasTemperature.requestTemperatures();
}

void LocalSensorTask::run()
{
	float t = dallasTemperature.getTempCByIndex(0);
	if (t == -127.0f)
	{
		temperature = 0.0f;
		logPrintfX(F("LST"), F("Sensor not found..."));
		dallasTemperature.requestTemperatures();
		sleep(10_s);
		return;
	}

	temperature = t;

	logPrintfX(F("LST"), F("T = %s deg C"), String(t, 1).c_str());
	dallasTemperature.requestTemperatures();
	sleep(10_s);
}

String getLocalTemperature(void* t)
{
	LocalSensorTask* lst = static_cast<LocalSensorTask*>(t);
	String p = "\x81 ";
	p += String(lst->temperature, 1);
	p += (char)0x80;
	p += "C";
	return p;
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

void handleLSTStatus(ESP8266WebServer& webServer, void* t)
{
	LocalSensorTask* lst = static_cast<LocalSensorTask*>(t);
	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString(F("LST Status")), ss);

	macroStringReplace(lstStatusPageFS, constString(String(lst->temperature)), ss);
	webServer.send(200, textHtml, ss.buffer);
}

static RegisterPackage r("LST", new LocalSensorTask, TaskDescriptor::SLOW,
		{
			PageDescriptor("lst", "Local Sensors", &handleLSTStatus),
		},
		{
			{getLocalTemperature, 2_s, 1, false}
		}
);
