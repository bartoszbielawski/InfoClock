/*
 * utils.h
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include "pgmspace.h"

extern int32_t timezone;

int operator"" _s(long double seconds);
int operator"" _s(unsigned long long int seconds);

const char* getDateTime();

class String;

String getTime();
String getDate();

uint32_t getUpTime();

const char* generateRandomUUID();


class AsyncClient;

void sendWSPacket_P(uint8_t header, uint16_t size, const uint8_t* key, PGM_P payload, AsyncClient* client);
//void sendWSPacket(uint8_t header, uint16_t size, const uint8_t* key, const char* payload, Client* client);

class __FlashStringHelper;

//void logPrintf(const __FlashStringHelper* format, ...);
void logPrintfX(const __FlashStringHelper* app, const __FlashStringHelper* format, ...);

template <class T>
T min(T a, T b)
{
		return a < b? a: b;
}

class String;

void readConfigFromFlash();
String readConfig(const String& name);
void   writeConfig(const String& name, const String& value);


int32_t getTimeZone();
uint32_t getUpTime();

String dataSource(const char* name);

#endif /* UTILS_H_ */
