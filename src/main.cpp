#include "Arduino.h"
#include <time.h>
#include "sntp.h"
#include "FS.h"

#include "config.h"

#include "tasks_utils.h"
#include "utils.h"
#include "OTA.hpp"
#include "SyslogSender.h"

#include "DisplayTask.hpp"

#include "BrexitCountdownDisplay.h"

#include "ArduinoOTA.h"

using namespace Tasks;


void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);

	//the filesystem is not ready yet - format it and save some settings
	if (!SPIFFS.begin())
	{
		SPIFFS.format();
		SPIFFS.begin();
		writeConfig(F("configPassword"), DEFAULT_PASSWORD);
		logPrintfX(F("MAIN"), F("Formatting file system, the default password is %s"), readConfig(F("configPassword")).c_str());
	}

	//readConfigFromFlash();

	setupTasks();

	logPrintfX(F("MAIN"), F("\n\n"));
	logPrintfX(F("MAIN"), F("MAC Address: %s"), WiFi.macAddress().c_str());

	auto& displayTask = getDisplayTask();

	displayTask.pushMessage("Initializing...", 2_s);
	displayTask.pushMessage(versionString, 0.4_s, true);
	displayTask.addRegularMessage({&displayTask, getBrexitDowncountMessage, 0.05_s, 1, true});

	timezone = readConfig(F("timezone")).toInt();
	syslogServer = readConfig(F("syslogServer"));

	configTime(timezone, 0, "pool.ntp.org", "time.nist.gov", "ntp3.pl");

	configureOTA();
}

void loop()
{
	scheduleTasks();
	ArduinoOTA.handle();
}
