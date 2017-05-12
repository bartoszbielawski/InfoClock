/*
 * AsyncLoggerTask.cpp
 *
 *  Created on: 10.05.2017
 *      Author: caladan
 */

#include "AsyncLoggerTask.h"
#include "SyslogSender.hxx"
#include "utils.h"

void AsyncLoggerTask::log(const char* taskName, const char* message)
{
	messages.emplace(taskName, message);
}

void AsyncLoggerTask::run()
{
	while (not messages.empty())
	{
		auto& msg = messages.front();
		//String c = msg.second;
		syslogSend(msg.first, msg.second.c_str());
		messages.pop();
	}
	_wait(1_s);
}

