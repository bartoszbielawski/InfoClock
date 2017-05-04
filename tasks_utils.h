/*
 * tasks_utils.h
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#ifndef TASKS_UTILS_H_
#define TASKS_UTILS_H_

#include "task.hpp"

#include "WebServerTask.h"
#include "DisplayTask.hpp"

struct TaskDescriptor
{
		const static uint8_t ENABLED = 1;
		const static uint8_t CONNECTED = 2;
		const static uint8_t SLOW = 3;

		TaskDescriptor(Tasks::Task* task, uint8_t flags):
			task(task), flags(flags){}

		Tasks::Task* task;
		uint8_t flags;
};

struct RegisterTask
{
		RegisterTask(Tasks::Task* t, uint8_t flags = 0);
};


struct RegisterPage
{
		RegisterPage(const String& url, const String& label, std::function<void(ESP8266WebServer&)> ph);
};

void setupTasks();

Tasks::Task* addTask(Tasks::Task* t, uint8_t flags = 0);
void addTask(const TaskDescriptor& td);
void scheduleTasks();

WebServerTask& getWebServerTask();
DisplayTask& getDisplayTask();

extern bool slowTaskCanExecute;


#endif /* TASKS_UTILS_H_ */
