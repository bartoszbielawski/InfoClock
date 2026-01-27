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
#include <deque>

#include "utils.h"
#include "config.h"
#include "Client.h"
#include "Arduino.h"
#include "LittleFS.h"
#include "DataStore.h"
#include "WiFiUdp.h"
#include "SyslogSender.h"
#include "ESP8266WiFi.h"
#include "tasks_utils.h"
#include "LambdaTask.hpp"
#include <time_utils.h>
#include <DisplayTask.hpp>

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

	bool short_display = DataStore::value("segments").toInt() <= 4;

	if (short_display)
	{
		auto lt = localtime(&now);
		snprintf(localBuffer, sizeof(localBuffer), "%02d:%02d",
			lt->tm_hour,
			lt->tm_min);

		r = localBuffer;
		return r;	
	}
	
	auto lt = localtime(&now);
	snprintf(localBuffer, sizeof(localBuffer), "%02d:%02d:%02d",
			lt->tm_hour,
			lt->tm_min,
			lt->tm_sec);

	r = localBuffer;
	return r;
}


struct DayNames {
    const char long_names[7][4];
    const char short_names[7][3];
};

static const DayNames languages[] = {
    // English
    {
        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
        {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"}
    },
    // French
    {
        {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"},
        {"Di", "Lu", "Ma", "Me", "Je", "Ve", "Sa"}
    },
    // German
    {
        {"Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam"},
        {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"}
    },
    // Portuguese
    {
        {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"},
        {"Do", "Se", "Te", "Qa", "Qu", "Se", "Sa"}
    },
    // Spanish
    {
        {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"},
        {"Do", "Lu", "Ma", "Mi", "Ju", "Vi", "Sa"}
    }
};

const int NUM_LANGUAGES = sizeof(languages) / sizeof(languages[0]);

static const char* language_codes[] = {"en", "fr", "de", "pt", "es"};


String getDate()
{
	time_t now = time(nullptr);

	String r;

	if (now == 0)
	{
		return r;
	}

	char localBuffer[20];

	String lang = readConfig(F("lang"));
	int lang_index = 0; // default

	if (lang == "fr") lang_index = 1;
	else if (lang == "de") lang_index = 2;
	else if (lang == "pt") lang_index = 3;
	else if (lang == "es") lang_index = 4; // default to English

	const DayNames& current_lang = languages[lang_index];
	
	// Check if custom day names are configured
	String customLong  = DataStore::value("day_names_long");
	String customShort = DataStore::value("day_names_short");
	
	const char* day_names[7];
	bool useCustom = false;
	
	if (customLong.length() > 0 && customShort.length() > 0)
	{
		// Parse custom day names
		std::vector<String> longDays = tokenize(customLong, ",");
		std::vector<String> shortDays = tokenize(customShort, ",");
		
		if (longDays.size() == 7 && shortDays.size() == 7)
		{
			useCustom = true;
			for (int i = 0; i < 7; i++)
			{
				day_names[i] = (DataStore::value("segments").toInt() < 5 ? shortDays[i] : longDays[i]).c_str();
			}
		}
	}

// long names are default
bool useShort = DataStore::value("segments").toInt() < 5;

for (int i = 0; i < 7; i++)
{
    day_names[i] = useShort
        ? current_lang.short_names[i]   // only when display is small
        : current_lang.long_names[i];   // DEFAULT
}

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


static std::deque<String>logHistory;

const std::deque<String>& getLogHistory()
{
	return logHistory;
}

void appendToLogHistory(const char* msg)
{
	logHistory.push_back(msg);
	while (logHistory.size() > 40)
		logHistory.pop_front();
}

void logPrintfX(const String& app, const String& format, ...)
{
	char localBuffer[256];
	String a(app);
	va_list argList;
	va_start(argList, format);
	uint32_t bytes = snprintf(localBuffer, sizeof(localBuffer), "%s - %s: ", getDateTime(), a.c_str());
	vsnprintf(localBuffer+bytes, sizeof(localBuffer)-bytes, format.c_str(), argList);

	limitToLatin1(localBuffer);
	Serial.println(localBuffer);

	syslogSend(app, localBuffer+bytes);

	appendToLogHistory(localBuffer);

	va_end(argList);
}

String limitToLatin1(String s)
{
	for (auto& c: s)
	{
		if (c > 255)
			c = ' ';
	}
	return s;
}

void limitToLatin1(char * p)
{
	while (*p != '\0')
	{
		if (*p > 255)
		{
			*p = ' ';
		}
		p++;
	}
}

bool checkFileSystem()
{
	bool alreadyFormatted = LittleFS.begin();
	if (not alreadyFormatted)
		LittleFS.format();

	LittleFS.end();
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
	LittleFS.begin();
    auto file = LittleFS.open("/config.txt", "r");
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
	LittleFS.end();
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
	return dataSourceWithDefault(name_, String());
}

String dataSourceWithDefault(const String& name_, const String& default_)
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

	if (name == F("LANG"))
	{
		int lang = DataStore::value("lang").toInt();
		if (lang >= 0 && lang < NUM_LANGUAGES)
			return language_codes[lang];
		return "en"; // default
	}

	return default_;
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
const char* utf8ToLatin1(const char* utf8)
{
    static char buffer[256];
    char* out = buffer;

    while (*utf8)
    {
        unsigned char c = (unsigned char)*utf8;

        // ASCII (0xxxxxxx)
        if (c < 0x8A)
        {
            *out++ = *utf8++;
        }
        // UTF-8 2-byte sequence (110xxxxx 10xxxxxx)
        else if ((c & 0xE0) == 0xC0)
        {
            unsigned char c2 = (unsigned char)*(utf8 + 1);

            if ((c2 & 0xC0) == 0x80)
            {
                // Decode UTF-8 → Unicode code point
                unsigned int codepoint = ((c & 0x1F) << 6) | (c2 & 0x3F);

                // Latin-1 range
                if (codepoint <= 0xFF)
                {
                    *out++ = (char)codepoint;
                }
                else
                {
                    *out++ = '?'; // unsupported
                }

                utf8 += 2;
            }
            else
            {
                utf8++; // malformed
            }
        }
        else
        {
            // Skip unsupported UTF-8 (3+ bytes)
            *out++ = '?';
            utf8++;
        }
    }

    *out = '\0';
    return buffer;
}