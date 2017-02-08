/*
 * WebServerTask.cpp
 *
 *  Created on: 10.01.2017
 *      Author: Bartosz Bielawski
 */


#include "Arduino.h"
#include "time.h"
#include "FS.h"


#include "WebServerTask.h"
#include "ESP8266WebServer.h"
#include "MacroStringReplace.h"
#include "config.h"
#include "DataStore.h"

#include "html/webpage.h"

#include "utils.h"

ESP8266WebServer webServer(80);

FlashStream pageHeaderFS(pageHeader);

static const char textPlain[] = "text/plain";
static const char textHtml[] = "text/html";

FlashStream statusFS(statusPage);

String dataSource(const char* name)
{
	logPrintf("Requesting %s...", name);

	String result = readConfig(name);
	if (result.length() > 0)
		return result;

	result = dataStore().value(name);
	if (result)
		return result;

	if (strcmp(name, "heap") == 0)
		return String(ESP.getFreeHeap()) + "B";

	return "?";
}


void handleStatus()
{
	StringStream ss(2048);

	macroStringReplace(pageHeaderFS, constString("Status"), ss);
	macroStringReplace(statusFS, dataSource, ss);

	webServer.send(200, textHtml, ss.buffer);
}


bool handleAuth()
{
	bool authed = webServer.authenticate("user", readConfig(F("configPassword")).c_str());
	if (!authed)
		webServer.requestAuthentication();

	return authed;
}

//TODO: remove
void handleReadParams()
{
	if (!handleAuth()) return;

	logPrintf("WebServer - Reading params...");
	auto dir = SPIFFS.openDir(F("/config"));

	String response;

	while (dir.next())
	{
		response += dir.fileName();
		response += " -> ";
		auto f = dir.openFile("r");
		response += f.readStringUntil('\n');
		response += '\n';
	}

	for (auto& key: dataStore().getKeys())
	{
		response += key;
		response += " -> ";
		response += dataStore().value(key);
		response += '\n';
	}

	webServer.send(200, textPlain, response);
}

FlashStream webmessageFS(webmessagePage);

void handleWebMessage()
{
	if (!handleAuth())
		return;

	auto wm = webServer.arg(F("webmessage"));
	if (wm.length())
	{
		dataStore().value("webmessage") = wm;
		dataStore().value("webmessageIP") = webServer.client().remoteIP().toString();
	}

	StringStream ss(2048);

	macroStringReplace(pageHeaderFS, constString("Webmessage"), ss);
	macroStringReplace(webmessageFS, dataSource, ss);

	webServer.send(200, textHtml, ss.buffer);
}


FlashStream generalSettingsFS(generalSettingsPage);

void handleGeneralSettings()
{
	if (!handleAuth()) return;

	auto submitted = webServer.arg(F("submitted"));
	if (submitted.length())
	{
		logPrintf("Saving data...");
		writeConfig(F("essid"), webServer.arg(F("essid")));

		//we don't send the current value so we have to check if it is present
		auto pwd = webServer.arg(F("wifiPassword"));
		if (pwd.length())
			writeConfig(F("wifiPassword"), pwd);

		writeConfig(F("timezone"), webServer.arg(F("timezone")));
	}

	StringStream ss(2048);

	macroStringReplace(pageHeaderFS, constString("WiFi Settings"), ss);
	macroStringReplace(generalSettingsFS, dataSource, ss);

	webServer.send(200, textHtml, ss.buffer);
}


FlashStream owmFS(owmPage);

void handleWeatherServiceConfig()
{
	if (!handleAuth()) return;

	auto location = webServer.arg(F("owmId"));
	auto key   = webServer.arg(F("owmKey"));
	auto period = webServer.arg(F("owmPeriod"));

	if (location.length()) 	writeConfig(F("owmId"), location);
	if (key.length()) 		writeConfig(F("owmKey"), key);
	if (period.length())	writeConfig(F("owmPerdio"), period);

	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("OWM Settings"), ss);
	macroStringReplace(owmFS, dataSource, ss);

	webServer.send(200, textHtml, ss.buffer);
}



WebServerTask::WebServerTask()
{
	reset();
}

void WebServerTask::reset()
{
	sleep(5_s);
	webServer.stop();
}




void WebServerTask::run()
{
	if (!started)
	{
		started = true;
		logPrintf("WebServerTask - configuring server");

		webServer.onNotFound([](){
			webServer.send(404, "text/plain", "Not found... :/");
		});

		webServer.on("/", [](){
			webServer.send(200, "text/html", FPSTR(mainPage));
		});

		webServer.on("/owm", &handleWeatherServiceConfig);
		webServer.on("/webmessage", &handleWebMessage);
		webServer.on("/settings", &handleGeneralSettings);

		webServer.on("/readParams", &handleReadParams);
		webServer.on("/status", 	&handleStatus);

		webServer.begin();

		logPrintf("WebServerTask - ready!");
		return;
	}

	webServer.handleClient();
}

WebServerTask::~WebServerTask()
{
}

