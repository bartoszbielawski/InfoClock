/*
 * WebServerTask.h
 *
 *  Created on: 10.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef WEBSERVERTASK_H_
#define WEBSERVERTASK_H_

#include "task.hpp"
#include "Arduino.h"

extern String webMessage;

class WebServerTask: public Tasks::Task {
public:
	WebServerTask();

	virtual void run();

	virtual void reset();

	virtual ~WebServerTask();

	bool started = false;

private:
	uint64_t cntr = 0;
};

#endif /* WEBSERVERTASK_H_ */
