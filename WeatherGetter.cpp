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
#include "CounterCRTP.hpp"

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
	taskName = "WG";
	taskName += counter;

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
	String idn(F("owmId"));
	String keyn(F("owmKey"));
	idn += counter;
	keyn += counter;
	auto location = readConfig(idn);
	auto key = readConfig(keyn);

	if (location.length() == 0 or key.length() == 0)
	{
		logPrintfX(taskName, F("Service not configured... "));
		sleep(60_s);
		return;
	}

	char localBuffer[256];
	snprintf_P(localBuffer, sizeof(localBuffer), urlTemplate, location.c_str(), key.c_str());

	logPrintfX(taskName, F("URL = %s"), localBuffer);

	HTTPClient httpClient;
	httpClient.begin(localBuffer);

	int httpCode = httpClient.GET();

	if (httpCode != HTTP_CODE_OK)
	{
		logPrintfX(taskName, F("HTTP failed with code %d"), httpCode);
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

	pressure = processPressure(results["/root/main/pressure"]).toInt();
	temperature = String(results["/root/main/temp"].c_str()).toFloat();
	localization = results["/root/name"].c_str();

	DataStore::value(F("OWM.Pressure")) = processPressure(results["/root/main/pressure"]);
	DataStore::value(F("OWM.Temperature")) = processTemperature(results["/root/main/temp"]);
	DataStore::value(F("OWM.Location")) = String(results["/root/name"].c_str());

	//print all we have acquired - useful for adding new fields
	for (const auto& e: results)
	{
		logPrintfX(taskName, F("%s = %s"), e.first.c_str(), e.second.c_str());
	}

	httpClient.end();

	String periodParamName(F("owmPeriod"));
	periodParamName += counter;
	int period = readConfig(periodParamName).toInt() * MS_PER_CYCLE;
	if (period == 0)
		period = 600_s;

	sleep(period);
}

static const char owmConfigPage[] PROGMEM = R"_(
<form method="post" action="owm" autocomplete="on">
<table>
<tr><th>OpenWeatherMap</th></tr>
<tr><td class="l">ID:</td><td><input type="text" name="owmId" value="$owmId$"></td></tr>
<tr><td class="l">Key:</td><td><input type="text" name="owmKey" value="$owmKet$"></td></tr>
<tr><td class="l">Refresh period (s):</td><td><input type="text" name="owmPeriod" value="$owmPeriod$"></td></tr>
<tr><td/><td><input type="submit"></td></tr>
</table></form></body></html>
)_";

FlashStream owmConfigPageFS(owmConfigPage);

void handleOWMConfig(ESP8266WebServer& webServer, void* t)
{
	if (!handleAuth(webServer)) return;

	WeatherGetter* wg = (WeatherGetter*)t;

	auto location = webServer.arg(F("owmId"));
	auto key   = webServer.arg(F("owmKey"));
	auto period = webServer.arg(F("owmPeriod"));

	auto configIdName = String(F("owmId"))+wg->getId();
	auto configKeyName = String(F("owmKey"))+wg->getId();
	auto configPeriodName = String(F("owmPeriod"))+wg->getId();

	if (location.length()) 	writeConfig(configIdName, location);
	if (key.length()) 		writeConfig(configKeyName, key);
	if (period.length())	writeConfig(configPeriodName, period);

	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("OWM Settings"), ss);

	std::map<String, String> m = {
			{F("owmId"), 		readConfig(configIdName)},
			{F("owmKey"), 		readConfig(configKeyName)},
			{F("owmPeriod"), 	readConfig(configPeriodName)},
	};

	macroStringReplace(owmConfigPageFS, mapLookup(m), ss);
	webServer.send(200, textHtml, ss.buffer);
}

static const char owmStatusPage[] PROGMEM = R"_(
<table>
<tr><th>$loc$</th></tr>
<tr><td class="l">Temperature:</td><td>$t$ &#8451;</td></tr>
<tr><td class="l">Pressure:</td><td>$p$ hPa</td></tr>
</table></form></body></html>
)_";

FlashStream owmStatusPageFS(owmStatusPage);

void handleOWMStatus(ESP8266WebServer& webServer, void* t)
{
	WeatherGetter* wg = static_cast<WeatherGetter*>(t);
	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("OWM Status"), ss);

	std::map<String, String> m =
	{
			{F("loc"), wg->localization},
			{F("t"), String(wg->temperature)},
			{F("p"), String(wg->pressure)}
	};
	macroStringReplace(owmStatusPageFS, mapLookup(m), ss);
	webServer.send(200, textHtml, ss.buffer);
}

static RegisterPackage r1("OWM1", new WeatherGetter, TaskDescriptor::CONNECTED,
{
	PageDescriptor("owmc1", "OWM Config", &handleOWMConfig),
	PageDescriptor("owms1", "OWM Status", &handleOWMStatus)
});

