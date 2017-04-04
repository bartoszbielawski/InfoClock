/*
 * tasks_utils.cpp
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#include <vector>
#include "tasks_utils.h"
#include "config.h"
#include "Arduino.h"

//ESP8266 raw API
extern "C" {
#include "user_interface.h"
}

using namespace Tasks;

static std::vector<Task*> tasks{};
static os_timer_t myTimer;

static void timerCallback(void*)
{
	updateSleep(tasks);
}

void setupTasks()
{
	os_timer_setfn(&myTimer, timerCallback, NULL);
	os_timer_arm(&myTimer, MS_PER_CYCLE, true);
}

Task* addTask(Task* task)
{
	tasks.emplace_back(task);
	return task;
}

void scheduleTasks()
{
	schedule(tasks);
}


