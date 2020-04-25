/*
* MessagesTask.h
*
*  Created on: 18.04.2020
*      Author: agorzawski
*/
#ifndef MESSAGESTASK_H_
#define MESSAGESTASK_H_

#include <Arduino.h>
#include <tasks.hpp>
#include <time.h>
#include <time_utils.h>
#include <utils.h>
#include <set>

const static DeltaTimePrecision allowedPrecisions[] = {DeltaTimePrecision::DAYS,
														DeltaTimePrecision::HOURS,
														DeltaTimePrecision::MINUTES,
														DeltaTimePrecision::SECONDS};

class MessagesTask: public Tasks::Task
{
	public:
		MessagesTask();
		virtual ~MessagesTask() = default;
		virtual void run();

	private:
    void updateFromConfig();
    String getMessages();
    String getMessage(String messageKey);
	String getOneStringFrom(std::vector<String> messages);

	std::vector<String> messageKeys;

    size_t messageCycleIndex = 0;
};

#endif /* MESSAGESTASK_H_ */
