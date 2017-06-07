/*
 * AsyncLoggerTask.h
 *
 *  Created on: 10.05.2017
 *      Author: Bartosz Bielawski
 */

#ifndef ASYNCLOGGERTASK_H_
#define ASYNCLOGGERTASK_H_

#include <task.hpp>
#include "Arduino.h"
#include <queue>

class AsyncLoggerTask: public Tasks::Task
{
	public:
		AsyncLoggerTask() = default;
		virtual ~AsyncLoggerTask() = default;

		void log(const char* taskName, const char* message);

		virtual void run();
		virtual bool checkCondition() {return not messages.empty();}

	private:
		std::queue<std::pair<String, String>> messages;
};

#endif /* ASYNCLOGGERTASK_H_ */
