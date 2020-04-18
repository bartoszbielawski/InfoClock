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
#include "DisplayTask.hpp"

#include "MessagesTask.h"

#include <LHCStatusReaderNew.h>
#include <LEDBlinker.h>
#include <MQTTTask.h>
#include <WeatherGetter.h>
#include <LocalSensorTask.h>
#include <SerialCommand.h>

//ESP8266 raw API
extern "C" {
#include "user_interface.h"
}

using namespace Tasks;

bool slowTaskCanExecute = false;

std::vector<TaskDescriptor>& getTasks()
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

void setupTasks()
{
	addTask(&WifiConnector::getInstance());
	addTask(&WebServerTask::getInstance());
	addTask(&DisplayTask::getInstance());

	addTask(new SerialCommandTask, 0);
	// LHC task seems to be broken in some strange way - with an exception in the HTTP client
	//addOptionalTask<LHCStatusReaderNew>(F("lhcEnabled"), TaskDescriptor::CONNECTED | TaskDescriptor::SLOW);
	addOptionalTask<LEDBlinker>(F("ledEnabled"), 0);
	addOptionalTask<WeatherGetter>(F("owmEnabled"), TaskDescriptor::SLOW | TaskDescriptor::CONNECTED);
	addOptionalTask<MQTTTask>(F("mqttEnabled"), TaskDescriptor::CONNECTED);
	addOptionalTask<LocalSensorTask>(F("lstEnabled"), TaskDescriptor::SLOW);
	addOptionalTask<MessagesTask>(F("messagesEnabled"), TaskDescriptor::ENABLED);

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

template <class T>
using TaskMemberWebCallback = void (T::*)(ESP8266WebServer&);

template <class T>
void registerPage(const String& url, const String& label, T* task, TaskMemberWebCallback<T> callback)
{
	WebServerTask::getInstance().registerPage(url, label,
	[task, callback](ESP8266WebServer& ws) {task->*callback(ws);});
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


void addRegularMessage(const DisplayState& ds)
{
	DisplayTask::getInstance().addRegularMessage(ds);
}
