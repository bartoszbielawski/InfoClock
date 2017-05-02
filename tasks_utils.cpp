/*
 * tasks_utils.cpp
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#include <vector>

#include "Arduino.h"
#include "ArduinoOTA.h"

#include "utils.h"
#include "tasks_utils.h"
#include "config.h"
#include "DataStore.h"

#include "WebServerTask.h"
#include "WifiConnector.h"

//ESP8266 raw API
extern "C" {
#include "user_interface.h"
}

using namespace Tasks;

static std::vector<TaskDescriptor>& getTasks()
		{
	static std::vector<TaskDescriptor> tasks;
	return tasks;
		}

static os_timer_t myTimer;

static void timerCallback(void*)
{
	for (auto& td: getTasks())
		updateSleep(td.task);
}

WifiConnector& getWifiConnector();
WebServerTask& getWebServerTask();
DisplayTask&   getDisplayTask();

void setupTasks()
{
	addTask(&getWifiConnector());
	addTask(&getWebServerTask());
	addTask(&getDisplayTask());

	os_timer_setfn(&myTimer, timerCallback, NULL);
	os_timer_arm(&myTimer, MS_PER_CYCLE, true);
}

void addTask(const TaskDescriptor& td)
{
	getTasks().emplace_back(td);
}

Tasks::Task* addTask(Tasks::Task* t, uint8_t flags)
{
	getTasks().emplace_back(t, flags);
	return t;
}


RegisterTask::RegisterTask(Tasks::Task* t, uint8_t flags)
{
	addTask(t, flags);
}

void scheduleTasks()
{
	for (auto& td: getTasks())
	{
		schedule(td.task);
	}
}


static void wifiConnectorCallback(WifiConnector::States state)
{
	switch (state)
	{
		case WifiConnector::States::NONE:
			//suspend connected tasks
			for (const auto& td: getTasks())
			{
				if (td.flags & TaskDescriptor::CONNECTED)
					td.task->suspend();
			}
			return;

		case WifiConnector::States::AP:
		{
			getWebServerTask().reset();

			getDisplayTask().pushMessage(F("AP mode"), 10_s);
			String ip = WiFi.softAPIP().toString();
			DataStore::value("ip") = ip;
			logPrintfX(F("WC"), F("IP = %s"), ip.c_str());
			return;
		}

		case WifiConnector::States::CLIENT:
		{
			getWebServerTask().reset();

			for (const auto& td: getTasks())
			{
				if (td.flags & TaskDescriptor::CONNECTED)
				{
					td.task->reset();
					td.task->resume();
				}
			}

			getDisplayTask().pushMessage(readConfig(F("essid")), 0.4_s, true);
			String ip = WiFi.localIP().toString();
			getDisplayTask().pushMessage(ip, 0.1_s, true);

			DataStore::value(F("ip")) = ip;
			logPrintfX(F("WC"), F("IP = %s"), ip.c_str());

			ArduinoOTA.begin();
			break;
		}
	}
}

WifiConnector& getWifiConnector()
{
	static WifiConnector wifiConnector(wifiConnectorCallback);
	return wifiConnector;
}

WebServerTask& getWebServerTask()
{
	static WebServerTask webServerTask;
	return webServerTask;
}

DisplayTask& getDisplayTask()
{
	static DisplayTask displayTask(DISPLAYS);
	return displayTask;
}
