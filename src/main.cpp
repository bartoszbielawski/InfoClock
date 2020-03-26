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

#include "StatelessMessages.h"

#include "ArduinoOTA.h"

using namespace Tasks;


void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);

	delay(5000);
	
	checkFileSystem();
	
	readConfigFromFS();
	
	setupTasks();

	logPrintfX(F("MAIN"), F("MAC Address: %s"), WiFi.macAddress().c_str());

	auto& displayTask = getDisplayTask();

	displayTask.pushMessage("Initializing...", 2_s);
	displayTask.pushMessage(versionString, 0.4_s, true);
	displayTask.addRegularMessage({&displayTask, getMessage, 0.05_s, 1, true});

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

