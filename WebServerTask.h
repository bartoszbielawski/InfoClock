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
#include "ESP8266WebServer.h"

//extern String webMessage;

class WebServerTask: public Tasks::Task {
public:
	WebServerTask();

	virtual void run();

	virtual void reset();

	virtual ~WebServerTask();


	void registerPage(const String& url, const String& label, std::function<void(ESP8266WebServer&)> ph);

	bool started = false;

private:
	ESP8266WebServer webServer;
	std::vector<std::pair<String, String>> registeredPages;
};

#endif /* WEBSERVERTASK_H_ */
