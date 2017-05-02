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

extern "C" {
#include "user_interface.h"
}

int operator"" _s(long double seconds) {return seconds * 1000 / MS_PER_CYCLE;}
int operator"" _s(unsigned long long int seconds) {return seconds * 1000 / MS_PER_CYCLE;}


//static char dateTimeBuffer[] = "00/00/00 00:00:00";
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

void sendWSPacket_P(uint8_t header, uint16_t size, const uint8_t* key, PGM_P payload, Client* client)
{
	//header, length, key, payload
	uint32_t totalSize = 1 + (size >= 0x7E ? 3: 1) + 4 + size;
	//logPrintf("SWS: total packet size: %d", totalSize);

	std::unique_ptr<uint8_t[]> ptr(new uint8_t[totalSize]);
	uint8_t* pckt = ptr.get();

	//logPrintf("SWS: header...");
	*pckt++ = header;

	//logPrintf("SWS: len...");
	if (size >= 0x7E)
	{
		*pckt++ = 0xFE;
		*pckt++ = size >> 8;
		*pckt++ = size & 0xFF;
	}
	else
	{
		*pckt++ = (size & 0x7F) | 0x80;
	}

	//logPrintf("SWS: key...");
	for (int i = 0; i < 4; i++)
		*pckt++ = key[i];

	//logPrintf("SWS: body...");
	for (int i = 0; i < size; i++)
		*pckt++ = pgm_read_byte(payload+i) ^ key[i % 4];

	client->write(ptr.get(), totalSize);
	//logPrintf("SWS: done!...");
}

void sendWSPacket(uint8_t header, uint16_t size, const uint8_t* key, const char* payload, Client* client)
{
	client->write(header);

	if (size >= 0x7E)
	{
		client->write(0xFE);
		client->write(size >> 8);
		client->write(size & 0xFF);
	}
	else
	{
		client->write((size & 0x7F) | 0x80);
	}

	client->write(key, 4);

	for (int i = 0; i < size; i++)
	{
		client->write(payload[i] ^ key[i % 4]);
	}
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


void logPrintf(const char* format, ...)
{
	char localBuffer[256];
	va_list argList;
	va_start(argList, format);
	uint32_t bytes = snprintf(localBuffer, sizeof(localBuffer), "%s - ", getDateTime());
	vsnprintf(localBuffer+bytes, sizeof(localBuffer)-bytes, format, argList);
	Serial.println(localBuffer);

	syslogSend(F("XX"), localBuffer+bytes);

	va_end(argList);
}

//void logPrintf(const __FlashStringHelper* format, ...)
//{
//	char localBuffer[256];
//	va_list argList;
//	va_start(argList, format);
//	uint32_t bytes = snprintf(localBuffer, sizeof(localBuffer), "%s - ", getDateTime());
//	vsnprintf_P(localBuffer+bytes, sizeof(localBuffer)-bytes, (PGM_P)format, argList);
//	Serial.println(localBuffer);
//
//	syslogSend(localBuffer+bytes);
//
//	va_end(argList);
//}

void logPrintfX(const __FlashStringHelper* app, const __FlashStringHelper* format, ...)
{
	char localBuffer[256];
	String a(app);
	va_list argList;
	va_start(argList, format);
	uint32_t bytes = snprintf(localBuffer, sizeof(localBuffer), "%s - %s: ", getDateTime(), a.c_str());
	vsnprintf_P(localBuffer+bytes, sizeof(localBuffer)-bytes, (PGM_P)format, argList);
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
	//logPrintf("RC: %s = %s", name.c_str(), value.c_str());
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
