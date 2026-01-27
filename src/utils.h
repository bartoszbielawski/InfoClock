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
#include <deque>

extern int32_t timezone;
class String;

uint16_t operator"" _s(long double seconds);
uint16_t operator"" _s(unsigned long long int seconds);

// Clock and Date utils

const char* getDateTime();
String getTime();
String getDate();
uint32_t getUpTime();
int32_t getTimeZone();


// Logging helpers
const std::deque<String>& getLogHistory();
void logPrintfX(const String& app, const String& format, ...);
String limitToLatin1(String s);
void limitToLatin1(char * s);

// Configuration helpers
void readConfigFromFS();
bool checkFileSystem();
String readConfigWithDefault(const String& name, const String& def);
String readConfig(const String& name);

String dataSource(const String& name_);
String dataSourceWithDefault(const String& name_, const String& default_);

namespace fs
{
	class File;
};

String readLine(fs::File& file);
std::pair<String, String> splitLine(String& line);
std::vector<String> tokenize(const String& s, const String& sep_str);

// Other random utils
const char* generateRandomUUID();
void rebootClock();

template <class T>
T min(T a, T b)
{
		return a < b? a: b;
}



#endif /* UTILS_H_ */
// Message correcter
const char* utf8ToLatin1(const char* utf8);