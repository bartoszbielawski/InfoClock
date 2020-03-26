/*
 * web_utils.cpp
 *
 *  Created on: 04.05.2017
 *      Author: caladan
 */



#include "web_utils.h"
#include "utils.h"
#include "ESP8266WebServer.h"
#include "config.h"
#include <html/webpage.h>
#include <WebServerTask.h>

FlashStream pageHeaderFS(pageHeader);

const char textPlain[] = "text/plain";
const char textHtml[] = "text/html";


bool handleAuth(ESP8266WebServer& webServer)
{
	bool authed = webServer.authenticate(DEFAULT_USER, readConfig(F("configPassword")).c_str());
	if (!authed)
		webServer.requestAuthentication();

	return authed;
}

void registerPage(const String& url, const String& label, std::function<void(ESP8266WebServer&)> ph)
{
	WebServerTask::getInstance().registerPage(url, label, ph);
}