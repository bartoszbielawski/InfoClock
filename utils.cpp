/*
 * utils.cpp
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */
#include <time.h>
#include <stdio.h>

#include "utils.h"
#include "config.h"
#include "Client.h"
#include "Arduino.h"
#include "FS.h"

extern "C" {
#include "user_interface.h"
}

int operator"" _s(long double seconds) {return seconds * 1000 / MS_PER_CYCLE;}
int operator"" _s(unsigned long long int seconds) {return seconds * 1000 / MS_PER_CYCLE;}


//static char dateTimeBuffer[] = "00/00/00 00:00:00";
static char dateTimeBuffer[20] = "Initializing.....";

static time_t previousDateTime = 0;

const char* getDateTime()
{
	time_t now = time(nullptr);
	if (now == previousDateTime)
		return dateTimeBuffer;

	auto lt = localtime(&now);
	snprintf(dateTimeBuffer, 32, "%02d/%02d/%02d %02d:%02d:%02d",
			lt->tm_year-100,
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
		client->write(pgm_read_byte(payload+i) ^ key[i % 4]);
	}
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


void logPrintf(char* format, ...)
{
	char localBuffer[128];
	va_list argList;
	va_start(argList, format);
	Serial.printf("%s - ", getDateTime());
	vsnprintf(localBuffer, sizeof(localBuffer), format, argList);
	Serial.println(localBuffer);
	va_end(argList);
}

void logPrintf(const __FlashStringHelper* format, ...)
{
	char localBuffer[128];
	va_list argList;
	va_start(argList, format);
	Serial.printf("%s - ", getDateTime());
	vsnprintf_P(localBuffer, sizeof(localBuffer), (PGM_P)format, argList);
	Serial.println(localBuffer);
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

String readConfig(const String& name)
{
	auto f = SPIFFS.open(String(F("/config/")) +name, "r");
	auto result = f.readStringUntil('\n');
	return result;
}

void writeConfig(const String& name, const String& value)
{
	auto f = SPIFFS.open(String(F("/config/")) +name, "w+");
	f.print(value);
	f.print('\n');
}


void saveToRTC(uint32_t address, uint32_t data)
{
	//	system_rtc_mem_write((address >> 2) + 64, &data, 4);
}

uint32_t readFromRTC(uint32_t address)
{
	//	uint32_t result = 0;
	//	system_rtc_mem_read((address >> 2) + 64, &data, 4);
	//	return result;
}

int32_t getTimeZone()
{
	return readConfig("timezone").toInt();
}
