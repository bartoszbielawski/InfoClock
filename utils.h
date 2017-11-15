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

class __FlashStringHelper;

void logPrintfX(const String& app, const String& format, ...);

//void logPrintfX(const __FlashStringHelper* app, const __FlashStringHelper* format, ...);
//void logPrintfX(const __FlashStringHelper* app, const char* format, ...);

template <class T>
T min(T a, T b)
{
		return a < b? a: b;
}

class String;

void readConfigFromFlash();
String readConfigWithDefault(const String& name, const String& def);
String readConfig(const String& name);
void   writeConfig(const String& name, const String& value);


void rebootClock();

int32_t getTimeZone();
uint32_t getUpTime();

String dataSource(const char* name);

#endif /* UTILS_H_ */
