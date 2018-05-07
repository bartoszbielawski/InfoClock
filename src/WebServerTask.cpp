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
#include "ESP8266HTTPUpdateServer.h"
#include "ArduinoOTA.h"

#include "MacroStringReplace.h"
#include "config.h"
#include "DataStore.h"

#include "html/webpage.h"

#include "utils.h"
#include "tasks_utils.h"
#include "web_utils.h"
#include "LambdaTask.hpp"



WebServerTask::WebServerTask():
		webServer(80)
{
	reset();
	DisplayState ds{this, [this]() {return webmessage;}, 0.05_s, 1, true};
	getDisplayTask().addRegularMessage(ds);
}



void WebServerTask::registerPage(const String& url, const String& label,
		std::function<void(ESP8266WebServer& ws)> ph)
{
	registeredPages.emplace_back(url, label);
	String newUrl("/");
	newUrl += url;
	std::function<void(void)> f = [this, ph] () {ph(webServer);};
	webServer.on(newUrl.c_str(), f);
}

void WebServerTask::reset()
{
	webServer.stop();
	sleep(5_s);
	started = false;

	webmessage = "";
	webmessageIP = "";
}


void WebServerTask::run()
{
	if (!started)
	{
		started = true;
		logPrintfX(F("WST"), F("Configuring server"));

		webServer.onNotFound([this](){
			webServer.send(404, "text/plain", "Not found... :/");
		});

		webServer.on("/", [this](){
			handleMainPage();
		});

		webServer.on("/webmessage", [this](){handleWebMessage();});
		webServer.on("/settings", [this] (){handleGeneralSettings();});

		webServer.on("/status", 	[this](){handleStatus();});
		webServer.on("/reset", [this]{handleReset();});

		webServer.begin();

		logPrintfX(F("WST"), F("Ready!"));
		return;
	}

	webServer.handleClient();
	delay(0);
}

String WebServerTask::generateLinks()
{
	String results;

	for (auto& p: registeredPages)
	{
		results += "<a href=\"";
		results += p.first;
		results += "\">";
		results += p.second;
		results += "</a>\n";
	}

	return results;
}

FlashStream mainPageFS(mainPage);

void WebServerTask::handleMainPage()
{
	StringStream ss(2048);
	macroStringReplace(mainPageFS, [this] (const char*) {return generateLinks();}, ss);
	webServer.send(200, textHtml, ss.buffer);
}

FlashStream statusFS(statusPage);

void WebServerTask::handleStatus()
{
	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("Status"), ss);
	macroStringReplace(statusFS, dataSource, ss);
	webServer.send(200, textHtml, ss.buffer);
}


void WebServerTask::handleReset()
{
	if (!handleAuth(webServer)) return;

	rebootClock();

	webServer.sendHeader("Location", String("/"), true);
	webServer.send(302, "text/plain", "");
}


FlashStream webmessageFS(webmessagePage);

void WebServerTask::handleWebMessage()
{
	if (!handleAuth(webServer))
		return;

	auto wm = webServer.arg(F("webmessage"));
	if (wm.length())
	{
		webmessage = wm;
		webmessageIP = webServer.client().remoteIP().toString();
		logPrintfX(F("WST"),
				   F("New webmessage %s from %s"),
				   wm.c_str(),
				   webmessageIP.c_str());
	}

	StringStream ss(2048);

	macroStringReplace(pageHeaderFS, constString(F("Webmessage")), ss);
	macroStringReplace(webmessageFS, constString(webmessage), ss);

	webServer.send(200, textHtml, ss.buffer);
}


FlashStream generalSettingsFS(generalSettingsPage);

void WebServerTask::handleGeneralSettings()
{
	if (!handleAuth(webServer)) return;

	auto submitted = webServer.arg(F("submitted"));
	if (submitted.length())
	{
		logPrintfX(F("WST"), F("Saving data..."));
		writeConfig(F("essid"), webServer.arg(F("essid")));

		//we don't send the current value so we have to check if it is present
		auto pwd = webServer.arg(F("wifiPassword"));
		if (pwd.length())
			writeConfig(F("wifiPassword"), pwd);

		pwd = webServer.arg(F("configPassword"));

		if (pwd.length())
			writeConfig(F("configPassword"), pwd);

		writeConfig(F("timezone"), webServer.arg(F("timezone")));

		auto ss = webServer.arg(F("syslogServer"));
		writeConfig(F("syslogServer"), ss);
		//the new syslog settings will be used after reboot
	}

	StringStream ss(2048);

	macroStringReplace(pageHeaderFS, constString("WiFi Settings"), ss);
	macroStringReplace(generalSettingsFS, dataSource, ss);

	webServer.send(200, textHtml, ss.buffer);
}

