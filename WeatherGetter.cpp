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
#include <MapCollector.hpp>
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

const static char urlTemplate[] PROGMEM = "http://api.openweathermap.org/data/2.5/weather?id=%d&APPID=%s&units=metric";

static const vector<String> prefixes{
	"main",
	"name"
};

bool jsonPathFilter(const string& key, const string& /*value*/)
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
	getWebServerTask().registerPage(F("owms"), F("OWM Status"), [this](ESP8266WebServer& ws) {handleStatus(ws);});
	getWebServerTask().registerPage(F("owmc"), F("OWM Config"), [this](ESP8266WebServer& ws) {handleConfig(ws);});

	//wait for the network
	suspend();
}

void WeatherGetter::reset()
{
	weathers.clear();

	String ids = readConfig(F("owmId"));
	apiKey = readConfig(F("owmKey"));

	//generate list of "weathers" to be checked
	int32_t from = 0;
	int32_t to;

	do
	{
		auto commaIndex = ids.indexOf(",", from);
		to = commaIndex == -1 ? ids.length(): commaIndex;

		String s = ids.substring(from, to);
		logPrintfX(F("WG"), "ID: %s", s.c_str());

		from = to+1;

		weathers.emplace_back(s.toInt());
	}
	while (from < ids.length());

	logPrintfX(F("WG"), "Found %d IDs", weathers.size());

	getDisplayTask().removeRegularMessages(this);

	for (int i = 0; i < weathers.size(); ++i)
	{
		logPrintfX(F("WG"), "Adding screen message %d", i);
		getDisplayTask().addRegularMessage({
						this,
						[this, i](){return getWeatherDescription(i);},
						0.05_s,
						1,
						true});
	}


	currentWeatherIndex = 0;
}

void WeatherGetter::run()
{
	//if no weathers are configured or apiKey is missing...
	if (weathers.size() == 0 or apiKey.length() == 0)
	{
		logPrintfX(F("WG"), F("Service not configured... "));
		sleep(60_s);
		return;
	}

	Weather& w = weathers[currentWeatherIndex];


	char localBuffer[256];
	snprintf_P(localBuffer, sizeof(localBuffer), urlTemplate, w.locationId, apiKey.c_str());

	logPrintfX(F("WG"), F("URL[%d] = %s"), currentWeatherIndex, localBuffer);

	currentWeatherIndex++;
	currentWeatherIndex %= weathers.size();

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

	w.pressure = atoi(results["/root/main/pressure"].c_str());
	w.temperature = atof(results["/root/main/temp"].c_str());
	w.location = results["/root/name"].c_str();

	//print all we have acquired - useful for adding new fields
	//	for (const auto& e: results)
	//	{
	//		logPrintfX(F("WG"), F("%s = %s"), e.first.c_str(), e.second.c_str());
	//	}

	httpClient.end();


	if (currentWeatherIndex != 0)
	{
		sleep(5_s);		//one readout every 5 seconds, then longer break...
		return;
	}

	int period = readConfig(F("owmPeriod")).toInt() * MS_PER_CYCLE;
	if (period == 0)
		period = 600_s;

	sleep(period);
}

static const char owmConfigPage[] PROGMEM = R"_(
<form method="post" action="owmc" autocomplete="on">
<table>
<tr><th>OpenWeatherMap</th></tr>
<tr><td class="l">ID:</td><td><input type="text" name="owmId" value="$owmId$"></td></tr>
<tr><td class="l">Key:</td><td><input type="text" name="owmKey" value="$owmKet$"></td></tr>
<tr><td class="l">Refresh period (s):</td><td><input type="text" name="owmPeriod" value="$owmPeriod$"></td></tr>
<tr><td/><td><input type="submit"></td></tr>
</table></form></body></html>
)_";

FlashStream owmConfigPageFS(owmConfigPage);

void WeatherGetter::handleConfig(ESP8266WebServer& webServer)
{
	if (!handleAuth(webServer)) return;

	auto location = webServer.arg(F("owmId"));
	auto key   = webServer.arg(F("owmKey"));
	auto period = webServer.arg(F("owmPeriod"));

	auto configIdName = String(F("owmId"));
	auto configKeyName = String(F("owmKey"));
	auto configPeriodName = String(F("owmPeriod"));

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

	//reload display tasks
	reset();
}

static const char owmStatusPage[] PROGMEM = R"_(
<tr><th>$loc$</th></tr>
<tr><td class="l">Temperature:</td><td>$t$ &#8451;</td></tr>
<tr><td class="l">Pressure:</td><td>$p$ hPa</td></tr>
)_";

static const char owmFooterPage[] PROGMEM = R"_(
</table></form></body>
<script>setTimeout(function(){window.location.reload(1);}, 15000);</script>
</html>
)_";

FlashStream owmStatusPageFS(owmStatusPage);
FlashStream owmFooterPageFS(owmFooterPage);

void WeatherGetter::handleStatus(ESP8266WebServer& webServer)
{
	StringStream ss(2048);
	logPrintfX(F("WG"), "OWM Status...");
	macroStringReplace(pageHeaderFS, constString(F("OWM Status")), ss);
	ss.print("<table>");

	for (auto& w: weathers)
	{
		//don't show it if we didn't get anything yet...
		if (!w.location.length())
			continue;

		std::map<String, String> m =
		{
				{F("loc"), w.location},
				{F("t"), String(w.temperature)},
				{F("p"), String(w.pressure)}
		};
		macroStringReplace(owmStatusPageFS, mapLookup(m), ss);
	}

	logPrintfX(F("WG"), "Sending footer...");
	macroStringReplace(owmFooterPageFS, [](const char*) {return String();}, ss);
	logPrintfX(F("WG"), "Total size: %d", ss.buffer.length());

	webServer.send(200, textHtml, ss.buffer);
}

String WeatherGetter::getWeatherDescription(uint32_t index)
{
	String r;
	if (index >= weathers.size())
		return r;

	const Weather& w = weathers[index];

	r.reserve(64);

	if (w.location.length() == 0)
		return String();

	r += w.location;
	r += ": ";
	r += (char)0x82;		//external temperature symbol
	r += String(w.temperature, 1);
	r += '\x80';
	r += 'C';
	r += " ";
	r += w.pressure;
	r += " hPa";
	return r;
}



static RegisterTask r1(new WeatherGetter, TaskDescriptor::CONNECTED | TaskDescriptor::SLOW);

