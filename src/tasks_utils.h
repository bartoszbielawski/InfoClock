/*
 * tasks_utils.h
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#ifndef TASKS_UTILS_H_
#define TASKS_UTILS_H_

#include <tasks.hpp>

#include "WebServerTask.h"
#include <DataStore.h>
#include <DisplayTask.hpp>

struct TaskDescriptor
{
		const static uint8_t ENABLED = 1;
		const static uint8_t CONNECTED = 2;
		const static uint8_t SLOW = 4;

		TaskDescriptor(Tasks::Task* task, uint8_t flags):
			task(task), flags(flags){}

		Tasks::Task* task;
		uint8_t flags;
};

using PageCallback = std::function<void(ESP8266WebServer&, void*)>;

struct PageDescriptor
{
		PageDescriptor(const char* url, const char* label, PageCallback callback):
			url(url), label(label), callback(callback) {}

		const char* url;
		const char* label;
		PageCallback callback;
};

void addRegularMessage(const DisplayState& ds);

void setupTasks();

std::vector<TaskDescriptor>& getTasks();
Tasks::Task* addTask(Tasks::Task* t, uint8_t flags = 0);
void addTask(const TaskDescriptor& td);
void scheduleTasks();

template <class T>
void addOptionalTask(const String& variableName, uint8_t flags)
{
	bool enabled = DataStore::valueOrDefault(variableName, "0").toInt();
	if (not enabled)
		return;

	addTask(new T, flags);
}

extern bool slowTaskCanExecute;


#endif /* TASKS_UTILS_H_ */
