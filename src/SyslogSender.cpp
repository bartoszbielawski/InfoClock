/*
 * SyslogSender.cpp
 *
 *  Created on: 02.05.2017
 *      Author: caladan
 */


#include "SyslogSender.h"
#include "WiFiUdp.h"
#include "DataStore.h"
#include "utils.h"
#include "ESP8266WiFi.h"

extern "C"
{
#include "user_interface.h"
}


//<14>1 YYYY-MM-DDTHH-MM-SSZ HOSTNAME APPNAME APPID MSGID SD MSG

WiFiUDP udp;

String syslogServer;

void syslogSend(const String& app, const char* msg)
{
	if (WiFi.status() != WL_CONNECTED)
		return;

	if (!syslogServer.length())
		return;

	if (udp.beginPacket(syslogServer.c_str(), 514))
	{
		char buffer[512];
		snprintf(buffer, 512,"<14>1 %s.00 %s %s - - - %s", getDateTime(), wifi_station_get_hostname(),  app.c_str(), msg);
		//copy this one and change udp to Serial to check what we're sending :)
		udp.printf(buffer);
		//Serial.println(buffer);

		udp.endPacket();
		yield();
	}
}

