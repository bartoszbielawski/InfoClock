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
#include "tasks_utils.h"
#include "MapCollector.hpp"
#include "WebServerTask.h"
#include "web_utils.h"
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
	//"wind",
	//"sys/sun",
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
		logPrintfX(F("WG"), F("Service not configured... "));
		sleep(60_s);
		return;
	}

	char localBuffer[256];
	snprintf_P(localBuffer, sizeof(localBuffer), urlTemplate, location.c_str(), key.c_str());

	logPrintfX(F("WG"), F("URL = %s"), localBuffer);

	HTTPClient httpClient;
	httpClient.begin(localBuffer);

	int httpCode = httpClient.GET();

	if (httpCode != HTTP_CODE_OK)
	{
		logPrintfX(F("WG"), F("HTTP failed with code %d"), httpCode);
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
		logPrintfX(F("WG"), F("%s = %s"), e.first.c_str(), e.second.c_str());
	}

	httpClient.end();

	int period = readConfig(F("owmPeriod")).toInt() * MS_PER_CYCLE;
	if (period == 0)
		period = 600_s;

	sleep(period);
}

static const char owmPage[] PROGMEM = R"_(
<form method="post" action="owm" autocomplete="on">
<table>
<tr><td class="wide">OpenWeatherMap</td></tr>
<tr><td class="label">ID:</td><td><input type="text" name="owmId" value="$owmId$"></td></tr>
<tr><td class="label">Key:</td><td><input type="text" name="owmKey" value="$owmKet$"></td></tr>
<tr><td class="label">Refresh period (s):</td><td><input type="text" name="owmPeriod" value="$owmPeriod$"></td></tr>
<tr><td/><td><input type="submit"></td></tr>
</table></form></body></html>
)_";

FlashStream owmFS(owmPage);

void handleWeatherServiceConfig(ESP8266WebServer& webServer)
{
	if (!handleAuth(webServer)) return;

	auto location = webServer.arg(F("owmId"));
	auto key   = webServer.arg(F("owmKey"));
	auto period = webServer.arg(F("owmPeriod"));

	if (location.length()) 	writeConfig(F("owmId"), location);
	if (key.length()) 		writeConfig(F("owmKey"), key);
	if (period.length())	writeConfig(F("owmPeriod"), period);

	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("OWM Settings"), ss);
	macroStringReplace(owmFS, dataSource, ss);

	webServer.send(200, textHtml, ss.buffer);
}

static const char owmPageStatus[] PROGMEM = R"_(
<table>
<tr><td class="wide">Weather</td></tr>
<tr><td class="label">Location:</td><td>$OWM.Location$</td></tr>
<tr><td class="label">Temperature:</td><td>$OWM.Temperature$</td></tr>
<tr><td class="label">Pressure:</td><td>$OWM.Pressure$</td></tr>
</table>
</body>
</html>
)_";

FlashStream owmPageStatusFS(owmPageStatus);

void handleOwmStatus(ESP8266WebServer& webServer)
{
	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("OWM Status"), ss);
	macroStringReplace(owmPageStatusFS, dataSource, ss);
	webServer.send(200, textHtml, ss.buffer);
}

//TODO: add OWM status
static RegisterTask rt(new WeatherGetter, TaskDescriptor::CONNECTED | TaskDescriptor::SLOW);
static RegisterPage rpConfig("owm", "OWM Config", &handleWeatherServiceConfig);
static RegisterPage rpStatus("owms", "OWM Status", &handleOwmStatus);
