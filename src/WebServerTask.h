/*
 * WebServerTask.h
 *
 *  Created on: 10.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef WEBSERVERTASK_H_
#define WEBSERVERTASK_H_

#include <vector>

#include <tasks.hpp>
#include "Arduino.h"
#include "ESP8266WebServer.h"

class WebServerTask: public Tasks::Task {
public:
	WebServerTask();

	virtual void run();

	virtual void reset();

	virtual ~WebServerTask() = default;


	void registerPage(const String& url, const String& label, std::function<void(ESP8266WebServer&)> ph);

	static WebServerTask& getInstance();

	bool started = false;

	String webmessage;
	String webmessageIP;

private:
	void handleMainPage();
	void handleStatus();
	void handleReset();
	void handleWebMessage();
	void handleGeneralSettings();
	void handleConfig();

	String generateLinks();

	ESP8266WebServer webServer;
	std::vector<std::pair<String, String>> registeredPages;



};

#endif /* WEBSERVERTASK_H_ */
