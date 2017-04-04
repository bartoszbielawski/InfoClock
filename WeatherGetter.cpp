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

/*
 * 2660646 - Geneva
 * 2659667 - Meyrin
 * 2974936 - Sergy
 * 6424612 - Ornex
 */

const static char urlTemplate[] PROGMEM = "http://api.openweathermap.org/data/2.5/weather?%s&APPID=%s";

//FIXME: flash?
const static char temperaturePath[] = "/root/main/temp";
const static char pressurePath[] = "/root/main/pressure";
const static char locationPath[] = "/root/name";

String processTemperature(const std::string& temperature)
{
	float f = atof(temperature.c_str());
	if (f < 5)		//really, less than 5 K? is the probe in LHC?
		return String();

	f -= 273.15f;
	f *= 10.0f;
	f = roundf(f);
	f /= 10.0f;

	String s(f, 1);
	String p = "To:";
	s = p + s + "$C";
	return s;
}

String processPressure(const std::string& pressure)
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



static void weatherGetterJSONCallback(const std::string& key, const std::string& value)
{
	if (key == temperaturePath)
	{
		DataStore::value(F("OWM.Temperature")) = processTemperature(value);
	}
	if (key == pressurePath)
	{
		DataStore::value(F("OWM.Pressure")) = processPressure(value);
	}
	if (key == locationPath)
	{
		DataStore::value(F("OWM.Location")) = value.c_str();
	}
}

WeatherGetter::WeatherGetter(): pathListener(&jsonParser)
{
	reset();
	pathListener.monitoredPaths().push_back(temperaturePath);
	pathListener.monitoredPaths().push_back(pressurePath);
	pathListener.monitoredPaths().push_back(locationPath);
	pathListener.setCallback(weatherGetterJSONCallback);
	sleep(30_s);
}

void WeatherGetter::reset()
{
	//TODO: reset dataStore values?
}

void WeatherGetter::run()
{
	auto location = readConfig(F("owmId"));
	auto key = readConfig(F("owmKey"));

	if (location.length() == 0 or key.length() == 0)
	{
		logPrintf("Weather service not configured... ");
		sleep(60_s);
		return;
	}

	char localBuffer[256];
	snprintf_P(localBuffer, sizeof(localBuffer), urlTemplate, location.c_str(), key.c_str());

	logPrintf("Weather Service: URL = %s", localBuffer);

	httpClient.begin(localBuffer);

	int httpCode = httpClient.GET();

	if (httpCode != HTTP_CODE_OK)
	{
		logPrintf("HTTP failed with code %d\n", httpCode);
		//report error, retry in 60 seconds
		sleep(60_s);
		httpClient.end();
		return;
	}

	String json = httpClient.getString();

	jsonParser.reset();
	jsonParser.setListener(&pathListener);
	pathListener.clear();

	//TODO: add proper error handling of json parser
	for (int  i = 0; i < json.length(); ++i)
	{
		jsonParser.parse(json[i]);
	}

	const auto& temperature = DataStore::value(F("OWM.Temperature"));
	const auto& pressure = DataStore::value(F("OWM.Pressure"));

	logPrintf("Weather refreshed: T = %s, p = %s", temperature.c_str(), pressure.c_str());

	httpClient.end();

	int period = readConfig("owmPeriod").toInt() * MS_PER_CYCLE;
	if (period == 0)
		period = 600_s;

	sleep(period);
}
