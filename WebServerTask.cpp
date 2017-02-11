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

static const char uptimeFormat[] PROGMEM = "%dd%dh%dm%ds";

String dataSource(const char* name)
{
	String result;

	if (DataStore::hasValue(name))
	{
		result = DataStore::value(name);
		if (result)
			return result;
	}

	if (strcmp(name, "heap") == 0)
		return String(ESP.getFreeHeap()) + " B";

	if (strcmp(name, "version") == 0)
		return versionString;

	if (strcmp(name, "essid") == 0)
		return WiFi.SSID();

	if (strcmp(name, "uptime") == 0)
	{
		uint32_t ut = getUpTime();
		uint32_t d = ut  / (24 * 3600);
		ut -= d * 24 * 3600;
		uint32_t h = ut / 3600;
		ut -= h * 3600;
		uint32_t m = ut / 60;
		ut -= m * 60;
		uint32_t s = ut;

		char buf[64];

		snprintf_P(buf, sizeof(buf), uptimeFormat, d, h, m, s);

		return String(buf);
	}

	result = readConfig(name);
	if (result)
		return result;

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

	for (auto& key: DataStore::availableKeys())
	{
		response += key;
		response += " -> ";
		response += DataStore::value(key);
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
		DataStore::value(F("webmessage")) = wm;
		DataStore::value(F("webmessageIP")) = webServer.client().remoteIP().toString();
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
	if (period.length())	writeConfig(F("owmPeriod"), period);

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
	sleep(5_s);
	started = false;
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
	delay(0);
}

WebServerTask::~WebServerTask()
{
}

