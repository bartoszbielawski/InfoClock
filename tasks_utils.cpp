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

bool slowTaskCanExecute = false;

static std::vector<TaskDescriptor>& getTasks()
				{
	static std::vector<TaskDescriptor> tasks;
	return tasks;
				}

static os_timer_t myTimer;

static void timerCallback(void*)
{
	for (auto& td: getTasks())
		updateSleepSingle(td.task);
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
	task = t;
}

RegisterPage::RegisterPage(const String& url, const String& label, std::function<void(ESP8266WebServer&)> ph)
{
	getWebServerTask().registerPage(url, label, ph);
}

RegisterPackage::RegisterPackage(const char* name, Tasks::Task* t, uint8_t flags,
						 std::initializer_list<PageDescriptor> pages,
						 std::initializer_list<DisplayLineDescriptor> displayLines)
{
	addTask(t, flags);
	for (auto& pd: pages)
	{
		getWebServerTask().registerPage(pd.url, pd.label, [t, pd] (ESP8266WebServer& w) {
			pd.callback(w, t);
		});
	}

	for (auto& dld: displayLines)
	{
		DisplayState ds = {t, [t, dld](){return dld.provider(t);}, dld.period, dld.cycles, dld.scrolling};
		getDisplayTask().addRegularMessage(ds);
	}

	//add after each...
	getDisplayTask().addRegularMessage({t, getTime, 1_s, 5,	false});
}


void scheduleTasks()
{
	for (auto& td: getTasks())
	{
		bool slow = td.flags & TaskDescriptor::SLOW;

		//schedule(td.task);

		if (!slow)
		{
			scheduleSingle(td.task);
			continue;
		}

		//it is slow but the token is there
		if (td.task->getState() == Tasks::State::READY)
		{
			//when it's ready and it can be executed now - do it!
			if (slowTaskCanExecute)
			{

				//logPrintfX(F("TS"), F("Executing slow task..."));
				td.task->run();
				slowTaskCanExecute = false;
				continue;
			}

			//otherwise delay by 0.1s
			//logPrintfX(F("TS"), F("Delaying slow task execution..."));
			td.task->sleep(0.1_s);
		}
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

			getDisplayTask().pushMessage(readConfig(F("essid")), 0.4_s, true);
			String ip = WiFi.localIP().toString();
			getDisplayTask().pushMessage(ip, 0.1_s, true);

			DataStore::value(F("ip")) = ip;
			logPrintfX(F("WC"), F("IP = %s"), ip.c_str());

			ArduinoOTA.begin();

			for (const auto& td: getTasks())
			{
				if (td.flags & TaskDescriptor::CONNECTED)
				{
					td.task->reset();
					td.task->resume();
				}
			}
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
