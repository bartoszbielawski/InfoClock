/*
 * WifiConnector.cpp
 *
 *  Created on: 02.05.2017
 *      Author: caladan
 */


#include "WifiConnector.h"
#include "pgmspace.h"
#include "Arduino.h"

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
	WiFi.mode(WIFI_STA);

	String pwd   = readConfig(F("wifiPassword"));

	logPrintfX(F("WC"), F("Running in CLIENT mode..."));
	logPrintfX(F("WC"), F("Connecting to %s - %s"), essid.c_str(), pwd.c_str());

	WiFi.begin(essid.c_str(), pwd.c_str());
	nextState = &WifiConnector::monitorClientStatus;
}


void WifiConnector::monitorClientStatus()
{
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


