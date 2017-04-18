#include "Arduino.h"
#include <time.h>
#include "sntp.h"
#include "FS.h"

#include "config.h"
#include "DisplayTask.hpp"
#include "task.hpp"
#include "tasks_utils.h"
#include "utils.h"
#include "DataStore.h"
#include "OTA.hpp"

#include "WifiConnector.h"
#include "WeatherGetter.h"
#include "WebServerTask.h"
#include "LHCStatusReader.h"
#include "LocalSensorTask.h"

#include "ArduinoOTA.h"

#include "MacroStringReplace.h"

using namespace Tasks;

class LedBlinker: public Task
{
	public:
		LedBlinker()
		{
			pinMode(LBLUE, OUTPUT);
		}

		virtual void run()
		{
			digitalWrite(LBLUE, s);
			sleep(s ? 2_s - 1: 1);
			s = !s;
		}

	private:
		bool s = false;
};


void connectionStateChanged(WifiConnector::States state);

WifiConnector wifiConnector(connectionStateChanged);

LedBlinker ledBlinker;
DisplayTask displayTask(DISPLAYS);
WeatherGetter weatherGetter;
WebServerTask webServerTask;
LHCStatusReader lhcStatusReader;
LocalSensorTask localSensorTask;

void connectionStateChanged(WifiConnector::States state)
{
	switch (state)
	{
		case WifiConnector::States::NONE:
			weatherGetter.suspend();
			webServerTask.suspend();
			lhcStatusReader.suspend();

			return;

		case WifiConnector::States::AP:
		{
			webServerTask.reset();
			webServerTask.resume();

			displayTask.pushMessage(F("AP mode"), 10_s);
			String ip = WiFi.softAPIP().toString();
			DataStore::value("ip") = ip;

			logPrintf("IP = %s", ip.c_str());
			return;
		}

		case WifiConnector::States::CLIENT:
		{
			weatherGetter.reset();
			weatherGetter.resume();

			webServerTask.reset();
			webServerTask.resume();

			lhcStatusReader.reset();
			lhcStatusReader.resume();

			displayTask.pushMessage(readConfig(F("essid")), 0.4_s, true);
			String ip = WiFi.localIP().toString();
			displayTask.pushMessage(ip, 0.5_s, true);

			DataStore::value("ip") = ip;
			logPrintf("IP = %s", ip.c_str());



			ArduinoOTA.begin();
			break;
		}
	}
}


void setup()
{
	Serial.begin(921600);

	//the filesystem is not ready yet - format it and save some settings
	if (!SPIFFS.begin())
	{
		SPIFFS.format();
		SPIFFS.begin();
		writeConfig(F("configPassword"), "password");
		logPrintf("Formatting filesystem, the default password is %s", readConfig(F("configPassword")).c_str());
	}

	//these tasks are always running
	addTask(&ledBlinker);
	addTask(&wifiConnector);
	addTask(&displayTask);
	addTask(&localSensorTask);

	//and these need to be suspended
	addTask(&weatherGetter)->suspend();
	addTask(&webServerTask)->suspend();
	addTask(&lhcStatusReader)->suspend();

	setupTasks();

	String macAddress = WiFi.macAddress();
	DataStore::value("mac") = macAddress;

	logPrintf("\n\n");
	logPrintf("MAC Address: %s", macAddress.c_str());

	displayTask.pushMessage("Initializing...", 2_s);
	displayTask.pushMessage(versionString, 0.4_s, true);

	configTime(getTimeZone(), 0, "pool.ntp.org", "time.nist.gov", "ntp3.pl");

	configureOTA();
}

void loop()
{
	scheduleTasks();
	ArduinoOTA.handle();
}
