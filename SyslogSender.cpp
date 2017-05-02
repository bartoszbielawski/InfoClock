/*
 * SyslogSender.cpp
 *
 *  Created on: 02.05.2017
 *      Author: caladan
 */


#include "SyslogSender.hxx"
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

void syslogSend(const __FlashStringHelper*  app, char* msg)
{
	if (WiFi.status() != WL_CONNECTED)
		return;

	if (!syslogServer.length())
		return;

	if (udp.beginPacket(syslogServer.c_str(), 514))
	{
		String a(app);

		//this is a fix for my NAS on which
		//syslog discards everything before first ":" after the time
		size_t len = strlen(msg);
		for (auto i = 0; i < len; ++i)
		{
			if (msg[i] == ':') msg[i] = ';';
		}

		//copy this one and change udp to Serial to check what we're sending :)
		udp.printf("<14>1 %s.00Z %s %s - - - %s", getDateTime(), wifi_station_get_hostname(),  a.c_str(), msg);

		udp.endPacket();
	}
}
