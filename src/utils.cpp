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
#include <utility>

#include "utils.h"
#include "config.h"
#include "Client.h"
#include "Arduino.h"
#include "FS.h"
#include "DataStore.h"
#include "WiFiUdp.h"
#include "SyslogSender.h"
#include "ESP8266WiFi.h"
#include "tasks_utils.h"
#include "LambdaTask.hpp"
#include <time_utils.h>
#include <DisplayTask.hpp>

#include <FS.h>

extern "C" {
#include "user_interface.h"
}

uint16_t operator"" _s(long double seconds) {return seconds * 1000 / MS_PER_CYCLE;}
uint16_t operator"" _s(unsigned long long int seconds) {return seconds * 1000 / MS_PER_CYCLE;}

static char dateTimeBuffer[] = "1970-01-01T00:00:00";

static uint32_t startUpTime = 0;

uint32_t getUpTime()
{
	if (!startUpTime)
		return 0;

	return	time(nullptr) - startUpTime;
}

String getTime()
{
	time_t now = time(nullptr);

	if (now < 1000000)
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


static const char long_day_names[][4] PROGMEM = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char short_day_names[][4] PROGMEM = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};


String getDate()
{
	time_t now = time(nullptr);

	String r;

	if (now == 0)
	{
		return r;
	}

	char localBuffer[20];

	static const auto day_names = DataStore::value("segments").toInt() <= 5 ? short_day_names: long_day_names;

	auto lt = localtime(&now);
	snprintf(localBuffer, sizeof(localBuffer), "%s %02d/%02d",
			day_names[lt->tm_wday],
			lt->tm_mday,
			lt->tm_mon+1);

	r = localBuffer;

	return r;
}





const char* getDateTime()
{
	static time_t previousDateTime = 0;
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



std::pair<String, String> splitLine(String& line)
{
    std::pair<String, String> result;

    //line.trim();

    if (line.length() == 0)
        return result;

    if (line[0] == '#')
        return result;

    auto pos = line.indexOf('=');   //find the first character

    if (pos == -1)
    {
        result.first = line;
        return result;
    }

    result.first = line.substring(0, pos);
    line.remove(0, pos+1);          //remove the equal sign as well
    result.second = line;
    return result;
}

void readConfigFromFS()
{
    logPrintfX("UTL", F("Reading configuration values from the flash..."));
    //the FS has to be initialized already...
	SPIFFS.begin();
    auto file = SPIFFS.open("/config.txt", "r");
    if (!file)
	{
		logPrintfX(F("UTL"), F("The file is missing, please create your own config using the web interface!"));
		return;
	}

	logPrintfX(F("UTL"), "File size: %zu", file.size());

	//remove all the data that's already present
	DataStore::clear();

    while (file.available())
    {
		String l = readLine(file);
	    auto p = splitLine(l);
        if (not p.second.length())
            continue;

        logPrintfX("UTL", F("Config: %s = '%s'"), p.first.c_str(), p.second.c_str());
		DataStore::value(p.first) = p.second;
    }
	SPIFFS.end();
}


String readConfigWithDefault(const String& name, const String& def)
{
	return DataStore::valueOrDefault(name, def);
}

String readConfig(const String& name)
{
	return DataStore::value(name);
}

int32_t timezone = 0;

String dataSource(const String& name_)
{
	String result;

	String name = name_;
	

	if (DataStore::hasValue(name))
	{
		result = DataStore::value(name);
		if (result)
			return result;
	}

	name.toUpperCase();

	if (name == F("IP"))
		return WiFi.localIP().toString();

	if (name ==  F("HEAP"))
		return String(ESP.getFreeHeap()) + " B";

	if (name == F("VERSION"))
		return versionString;

	if (name ==  F("BUILD"))
		return __DATE__ " - " __TIME__;

	if (name == F("ESSID"))
		return WiFi.SSID();

	if (name == F("MAC"))
		return WiFi.macAddress();

	if (name == F("UPTIME"))
	{
		return formatDeltaTime(getUpTime(), DeltaTimePrecision::SECONDS);
	}

	return "?";
}

void rebootClock()
{
	DisplayTask::getInstance().pushMessage("Rebooting...", 5_s, false);
	logPrintfX(F("WS"), F("Rebooting in 5 seconds..."));
	LambdaTask* lt = new LambdaTask([](){ESP.restart();});
	addTask(lt, TaskDescriptor::ENABLED);
	lt->sleep(5_s);
}

String readLine(fs::File& file)
{
	String result;

	while (file.available())
	{
		int c = file.read();
		if (c == '\n')
			return result;

		if (c == '\r')
			return result;

		//cast it, otherwise a number is appended - not a char
		result += (char)c;
	}

	return result;
}

std::vector<String> tokenize(const String& input, const String& sep_str)
{
	uint32_t from = 0;
	int32_t to;

	std::vector<String> results;

	do
	{
		auto commaIndex = input.indexOf(sep_str, from);
		to = commaIndex == -1 ? input.length(): commaIndex;

		String s = input.substring(from, to);
		from = to + 1; //TODO see with sep_str.len()

		results.emplace_back(s);
	}
	while (from < input.length());

	return results;
}
