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

class MessagesTask: public Tasks::Task
{
	public:
		MessagesTask();
		virtual ~MessagesTask() = default;
		virtual void run();

	private:
    void updateFromConfig(bool verbose);
    String getMessages();
    String getMessage(int messageIndex);
    int nbOfMessages = 0; //overiden on startup
    int messageCycleIndex = 0;

    DeltaTimePrecision defaultPrecision = DeltaTimePrecision::HOURS;
    bool defaultCounting = false;
    String defaultMessage = "...";
};

#endif /* MESSAGESTASK_H_ */
