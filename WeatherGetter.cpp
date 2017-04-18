/*
 * WeatherGetter.cpp
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */

#include "WeatherGetter.h"
#include "DataStore.h"
#include "config.h"
#include "utils.h"
#include "MapCollector.hpp"

/*
 * 2660646 - Geneva
 * 2659667 - Meyrin
 * 2974936 - Sergy
 * 6424612 - Ornex
 */

using namespace std;

const static char urlTemplate[] PROGMEM = "http://api.openweathermap.org/data/2.5/weather?%s&APPID=%s&units=metric";

String processTemperature(const string& temperature)
{
	float f = atof(temperature.c_str());

	f *= 10.0f;
	f = roundf(f);
	f /= 10.0f;

	String s(f, 1);
	String p;
	p += (char)0x82;
	p += ' ';
	p += s;
	p += '\x80';
	p += 'C';
	return p;
}

String processPressure(const string& pressure)
{
	if (pressure.empty())
	{
		return String();
	}

	float f = atof(pressure.c_str());
	f += 0.5;

	String result((int)f);
	result += " hPa";

	return result;
}


static const vector<String> prefixes{
	"main",
	"wind",
	"sys/sun",
	"name"
};

bool jsonPathFilter(const string& key)
{
	String s(key.c_str());

	for (auto& prefix: prefixes)
	{
		//"/root/" => first 6 chars
		if (s.startsWith(prefix, 6))
			return true;
	}

	return false;
}

WeatherGetter::WeatherGetter()
{
	reset();
	sleep(30_s);
}

void WeatherGetter::reset()
{
	DataStore::value(F("OWM.Temperature")) = String();
	DataStore::value(F("OWM.Pressure")) = String();
	DataStore::value(F("OWM.Location")) = String();
}

void WeatherGetter::run()
{
	auto location = readConfig(F("owmId"));
	auto key = readConfig(F("owmKey"));

	if (location.length() == 0 or key.length() == 0)
	{
		logPrintf(F("Weather service not configured... "));
		sleep(60_s);
		return;
	}

	char localBuffer[256];
	snprintf_P(localBuffer, sizeof(localBuffer), urlTemplate, location.c_str(), key.c_str());

	logPrintf(F("Weather Getter: URL = %s"), localBuffer);

	HTTPClient httpClient;
	httpClient.begin(localBuffer);

	int httpCode = httpClient.GET();

	if (httpCode != HTTP_CODE_OK)
	{
		logPrintf(F("HTTP failed with code %d"), httpCode);
		sleep(60_s);
		httpClient.end();
		return;
	}

	//fetch the response
	String json = httpClient.getString();

	//prepare the parser - provide filtering function
	MapCollector mc(jsonPathFilter);

	for (size_t  i = 0; i < json.length(); ++i)
	{
		mc.parse(json[i]);
	}

	auto& results = mc.getValues();

	DataStore::value(F("OWM.Pressure")) = processPressure(results["/root/main/pressure"]);
	DataStore::value(F("OWM.Temperature")) = processTemperature(results["/root/main/temp"]);
	DataStore::value(F("OWM.Location")) = String(results["/root/name"].c_str());

	//print all we have acquired - useful for adding new fields
	for (const auto& e: results)
	{
		logPrintf(F("Weather Getter: %s: %s"), e.first.c_str(), e.second.c_str());
	}

	httpClient.end();

	int period = readConfig(F("owmPeriod")).toInt() * MS_PER_CYCLE;
	if (period == 0)
		period = 600_s;

	sleep(period);
}
