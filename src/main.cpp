#include "Arduino.h"
#include <time.h>
#include "sntp.h"

#include "config.h"

#include "tasks_utils.h"
#include "utils.h"
#include "OTA.hpp"
#include "SyslogSender.h"

#include "DisplayTask.hpp"

#include "ArduinoOTA.h"

using namespace Tasks;


void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);

	checkFileSystem();

	readConfigFromFS();

	setupTasks();

	logPrintfX(F("MAIN"), F("MAC Address: %s"), WiFi.macAddress().c_str());

	auto& displayTask = DisplayTask::getInstance();

	displayTask.pushMessage("Initializing...", 2_s);
	displayTask.pushMessage(versionString, 0.4_s, true);

	syslogServer = readConfig(F("syslogServer"));

	configureOTA();
}

void loop()
{
	scheduleTasks();
	ArduinoOTA.handle();
}
