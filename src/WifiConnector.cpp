/*
 * WifiConnector.cpp
 *
 *  Created on: 02.05.2017
 *      Author: caladan
 */


#include "WifiConnector.h"
#include "pgmspace.h"
#include "Arduino.h"
#include <DataStore.h>
#include <tasks_utils.h>
#include <ArduinoOTA.h>

extern "C"
{
#include "user_interface.h"
};

WifiConnector::WifiConnector(Callback callback): TaskCRTP(&WifiConnector::lateInit), callback(callback)
{
}

void WifiConnector::lateInit()
{
	connectionTimeout = 60;
	if (callback)
		callback(States::NONE);

	auto essid = readConfig(F("essid"));

	if (essid.length() == 0)
		mainState = States::AP;

	switch (mainState)
	{
		case States::AP:
			initAP();
			break;
		default:
		case States::CLIENT:
			initSTA(essid);
			break;
	}
}


void WifiConnector::initAP()
{
	//run the AP
	WiFi.mode(WIFI_AP);
	WiFi.softAP("esp-display");

	//notify tasks we're AP now
	callback(States::AP);
	logPrintfX(F("WC"), F("Running in AP mode..."));

	//wake up in 60 seconds and try to reconnect once again as a client
	nextState = &WifiConnector::lateInit;
	mainState = States::CLIENT;
	sleep(300_s);
	return;
}

void WifiConnector::initSTA(const String& essid)
{
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	WiFi.hostname(readConfigWithDefault(F("hostname"), HOSTNAME));
	WiFi.mode(WIFI_STA);
	
	//set the lowest possible mode
	wifi_set_phy_mode(PHY_MODE_11N);

	String pwd   = readConfig(F("wifiPassword"));

	logPrintfX(F("WC"), F("Running in CLIENT mode..."));
	logPrintfX(F("WC"), F("Connecting to %s - %s"), essid.c_str(), pwd.c_str());

	WiFi.begin(essid.c_str(), pwd.c_str());
	nextState = &WifiConnector::monitorClientStatus;
}


void WifiConnector::monitorClientStatus()
{
	//WiFi.printDiag(Serial);
	auto status = WiFi.status();

	//check the status, notify about state change
	if (connected != (status == WL_CONNECTED))
	{
		connected = (status == WL_CONNECTED);
		if (callback)
		{
			callback(connected ? States::CLIENT: States::NONE);
		}
	}

	//handle timeout...
	connectionTimeout = connected ? 60: std::max(connectionTimeout-1, 0);

	if (connectionTimeout == 0)
	{
		logPrintfX(F("WC"), F("Timed out - falling back into AP mode..."));
		mainState = States::AP;
		nextState = &WifiConnector::lateInit;
		return;
	}
	sleep(connected ? 10_s: 1_s);
}

bool WifiConnector::getConnected() const {return connected;}

static void wifiConnectorCallback(WifiConnector::States state)
{
	switch (state)
	{
		case WifiConnector::States::NONE:
			//suspend connected tasks
			for (const auto& td: getTasks())
			{
				if (td.flags & TaskDescriptor::CONNECTED)
					td.task->suspend();
			}
			return;

		case WifiConnector::States::AP:
		{
			WebServerTask::getInstance().reset();

			DisplayTask::getInstance().pushMessage(F("AP mode"), 10_s);
			String ip = WiFi.softAPIP().toString();
			DataStore::value("ip") = ip;
			logPrintfX(F("WC"), F("IP = %s"), ip.c_str());
			return;
		}

		case WifiConnector::States::CLIENT:
		{
			WebServerTask::getInstance().reset();

			String tzs = readConfigWithDefault(F("timezone"), F("CET-1CEST,M3.5.0/2,M10.5.0/3"));			
			configTzTime(tzs.c_str(), "pool.ntp.org", "time.nist.gov", "ntp3.pl");

			DisplayTask::getInstance().pushMessage(readConfig(F("essid")), 0.4_s, true);
			String ip = WiFi.localIP().toString();
			DisplayTask::getInstance().pushMessage(ip, 0.1_s, true);

			DataStore::value(F("ip")) = ip;
			logPrintfX(F("WC"), F("IP = %s"), ip.c_str());

			ArduinoOTA.begin();

			for (const auto& td: getTasks())
			{
				if (td.flags & TaskDescriptor::CONNECTED)
				{
					td.task->reset();
					td.task->resume();
				}
			}
			break;
		}
	}
}


WifiConnector& WifiConnector::getInstance()
{
	static WifiConnector wifiConnector(wifiConnectorCallback);
	return wifiConnector;
}

