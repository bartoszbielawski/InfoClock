/*
 * utils.cpp
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */
#include <time.h>
#include <stdio.h>

#include <vector>
#include <memory>

#include "utils.h"
#include "config.h"
#include "Client.h"
#include "Arduino.h"
#include "FS.h"
#include "DataStore.h"
#include "WiFiUdp.h"
#include "SyslogSender.hxx"
#include "ESP8266WiFi.h"
#include "tasks_utils.h"

extern "C" {
#include "user_interface.h"
}

int operator"" _s(long double seconds) {return seconds * 1000 / MS_PER_CYCLE;}
int operator"" _s(unsigned long long int seconds) {return seconds * 1000 / MS_PER_CYCLE;}

static char dateTimeBuffer[] = "1970-01-01T00:00:00";

static uint32_t startUpTime = 0;

uint32_t getUpTime()
{
	return	time(nullptr) - startUpTime;
}

String getTime()
{
	time_t now = time(nullptr);

	if (now == 0)
	{
		return "??:??:??";
	}

	//this saves the first timestamp when it was nonzero (it's near start-up time)
	if (startUpTime == 0)
	{
		startUpTime = now;
	}

	String r;

	char localBuffer[10];

	auto lt = localtime(&now);
	snprintf(localBuffer, sizeof(localBuffer), "%02d:%02d:%02d",
			lt->tm_hour,
			lt->tm_min,
			lt->tm_sec);

	r = localBuffer;
	return r;
}


static const std::vector<const char*> dayNames{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
//static const std::vector<const char*> dayNames{"Nd", "Pn", "Wt", "Sr", "Cz", "Pt", "Sb"};


String getDate()
{
	time_t now = time(nullptr);

	String r;

	if (now == 0)
	{
		return r;
	}

	char localBuffer[20];

	auto lt = localtime(&now);
	snprintf(localBuffer, sizeof(localBuffer), "%s %02d/%02d",
			dayNames[lt->tm_wday],
			lt->tm_mday,
			lt->tm_mon+1);

	r = localBuffer;

	return r;
}



static time_t previousDateTime = 0;

const char* getDateTime()
{
	time_t now = time(nullptr);
	if (now == previousDateTime)
		return dateTimeBuffer;

	auto lt = localtime(&now);
	snprintf(dateTimeBuffer, 32, "%04d-%02d-%02dT%02d:%02d:%02d",
			lt->tm_year-100+2000,
			lt->tm_mon+1,
			lt->tm_mday,
			lt->tm_hour,
			lt->tm_min,
			lt->tm_sec);

	previousDateTime = now;
	return dateTimeBuffer;
}

const static char UUID_FORMAT[] PROGMEM = "%08x-%04x-4%03x-8%03x-%04x%08x";
static char UUID[36];

const char* generateRandomUUID()
{
	uint32_t r1 = os_random();
	uint32_t r2 = os_random();
	uint32_t r3 = os_random();
	uint32_t r4 = os_random();

	sprintf_P(UUID, UUID_FORMAT, r1, r2 >> 16, r2 & 0xFFF, r3 >> 20, r3 & 0xFFFF, r4);
	return UUID;
}


void logPPrintf(char* format, ...)
{
	char localBuffer[256];
	va_list argList;
	va_start(argList, format);
	Serial.printf("%s-%09u - ", getDateTime(), ESP.getCycleCount());
	vsnprintf(localBuffer, sizeof(localBuffer), format, argList);
	Serial.println(localBuffer);
	va_end(argList);
}


void logPrintfX(const String& app, const String& format, ...)
{
	char localBuffer[256];
	String a(app);
	va_list argList;
	va_start(argList, format);
	uint32_t bytes = snprintf(localBuffer, sizeof(localBuffer), "%s - %s: ", getDateTime(), a.c_str());
	vsnprintf(localBuffer+bytes, sizeof(localBuffer)-bytes, format.c_str(), argList);
	Serial.println(localBuffer);

	syslogSend(app, localBuffer+bytes);

	va_end(argList);
}


bool checkFileSystem()
{
	bool alreadyFormatted = SPIFFS.begin();
	if (not alreadyFormatted)
		SPIFFS.format();

	SPIFFS.end();
	return alreadyFormatted;
}

void readConfigFromFlash()
{
	SPIFFS.begin();
	auto dir = SPIFFS.openDir(F("/config"));

	while (dir.next())
	{
		auto f = dir.openFile("r");
		auto value = f.readStringUntil('\n');
		//skip the /config/ part of the path
		auto name = dir.fileName().substring(8);
		DataStore::value(name) = value;
		//logPrintf(F("RCFF: %s = %s"), name.c_str(), value.c_str());
	}
}

String readConfig(const String& name)
{
	auto value = DataStore::value(name);
	return value;
}

void writeConfig(const String& name, const String& value)
{
	auto f = SPIFFS.open(String(F("/config/")) +name, "w+");
	f.print(value);
	f.print('\n');
	f.close();
	DataStore::value(name) = value;
}


int32_t getTimeZone()
{
	return readConfig("timezone").toInt();
}

int32_t timezone = 0;

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

	if (strcmp(name, "mac") == 0)
		return WiFi.macAddress();

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

	return "?";
}
