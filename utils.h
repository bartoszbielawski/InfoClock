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

int operator"" _s(long double seconds);
int operator"" _s(unsigned long long int seconds);

const char* getDateTime();

const char* generateRandomUUID();

class Client;

void sendWSPacket_P(uint8_t header, uint16_t size, const uint8_t* key, PGM_P payload, Client* client);
void sendWSPacket(uint8_t header, uint16_t size, const uint8_t* key, const char* payload, Client* client);

void logPrintf(char* format, ...);

class __FlashStringHelper;

void logPrintf(const __FlashStringHelper* format, ...);

template <class T>
T min(T a, T b)
{
		return a < b? a: b;
}

class String;

bool checkFileSystem();
String readConfig(const String& name);
void   writeConfig(const String& name, const String& value);

int32_t getTimeZone();

void saveToRTC(uint32_t address, uint32_t data);
uint32_t readFromRTC(uint32_t address);


#endif /* UTILS_H_ */
