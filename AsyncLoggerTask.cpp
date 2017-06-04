/*
 * AsyncLoggerTask.cpp
 *
 *  Created on: 10.05.2017
 *      Author: Bartosz Bielawski
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
		syslogSend(msg.first, msg.second.c_str());
		messages.pop();
	}
	_wait(0.5_s);
}

