/*
 * WeatherGetter.cpp
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */

#include "WeatherGetter.h"
#include "config.h"
#include "utils.h"
#include "tasks_utils.h"
#include <MapCollector.hpp>
#include "web_utils.h"

/*
 * 2660646 - Geneva
 * 2659667 - Meyrin
 * 2974936 - Sergy
 * 6424612 - Ornex
 */

using namespace std;

const static char urlTemplate[] PROGMEM =
		"http://api.openweathermap.org/data/2.5/weather?id=%d&APPID=%s&units=metric&lang=%s";
const static char urlForecastTemplate[] PROGMEM =
		"http://api.openweathermap.org/data/2.5/forecast?id=%d&APPID=%s&units=metric&cnt=2&lang=%s";


/* forecast path
 /root/list/n/main/temp			--temperature
 /root/list/n/weather/0/main	--description
 /root/list/n/weather/0/description --
 */

static const vector<String> prefixes{
	"main/temp",
	"name",
	"list/1/main/temp",
	"list/1/weather/0/description"
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
	registerPage(F("owms"), F("OWM Status"), [this](ESP8266WebServer& ws) {handleStatus(ws);});

	addRegularMessage({this, [this](){return getWeatherDescription();}, 0.035_s, 1, true});

	//wait for the network
	suspend();
}

void WeatherGetter::reset()
{
	weathers.clear();

	String ids = readConfig(F("owmId"));
	apiKey = readConfig(F("owmKey"));

	//generate list of "weathers" to be checked
	uint32_t from = 0;
	int32_t to;

	do
	{
		auto commaIndex = ids.indexOf(",", from);
		to = commaIndex == -1 ? ids.length(): commaIndex;

		String s = ids.substring(from, to);
		logPrintfX(F("WG"), F("ID: %s"), s.c_str());

		from = to+1;

		weathers.emplace_back(s.toInt());
	}
	while (from < ids.length());

	logPrintfX(F("WG"), F("Found %d IDs"), weathers.size());

	currentWeatherIndex = 0;
}



int getHttpResponse(WiFiClient& wifiClient, HTTPClient& httpClient, MapCollector& mc, const char* url)
{
	httpClient.begin(wifiClient, url);

	int httpCode = httpClient.GET();

	if (httpCode != 200)
	{
		httpClient.end();
		return httpCode;
	}

	mc.reset();

	String json = httpClient.getString();
	for (size_t  i = 0; i < json.length(); ++i)
	{
		mc.parse(json[i]);
	}

	httpClient.end();

	return httpCode;
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
	logPrintfX(F("WG"), "Reading weather for id = %d", w.locationId);

	currentWeatherIndex++;
	currentWeatherIndex %= weathers.size();

	//locals needed for the rest of the code
	MapCollector mc(jsonPathFilter);
	WiFiClient wifiClient;
	HTTPClient httpClient;
	httpClient.setReuse(true);
	char localBuffer[256];
	int code = 0;

	while (true)
	{
		// before building URLs in WeatherGetter::run():
		String lang = readConfig(F("lang"));
		//get weather
		snprintf_P(localBuffer, sizeof(localBuffer), urlTemplate, w.locationId, apiKey.c_str(), lang.c_str());
		logPrintfX(F("WG"), "URL: %s", localBuffer);
		code = getHttpResponse(wifiClient, httpClient, mc, localBuffer);
		if (code != 200)
			break;

		auto& results = mc.getValues();
		w.temperature = atof(results["/root/main/temp"].c_str());
		w.location = results["/root/name"].c_str();

		//get forecast
		snprintf_P(localBuffer, sizeof(localBuffer), urlForecastTemplate, w.locationId, apiKey.c_str(), lang.c_str());
		logPrintfX(F("WG"), "URL: %s", localBuffer);
		code = getHttpResponse(wifiClient, httpClient, mc, localBuffer);
		if (code != 200)
			break;

		w.temperatureForecast = atof(results["/root/list/1/main/temp"].c_str());
		w.description = results["/root/list/1/weather/0/description"].c_str();

		//oops, forgot to break...
		break;
	}
	if (code != 200)
	{
		logPrintfX(F("WG"), F("HTTP failed with code %d"), code);
		sleep(600_s);
		return;
	}

	if (currentWeatherIndex != 0)
	{
		sleep(5_s);		//one readout every 5 seconds, then longer break...
		return;
	}

	int period = readConfig(F("owmPeriod")).toInt() * 1_s;
	if (period == 0)
		period = 600_s;

	sleep(period);
}

static const char owmStatusPage[] PROGMEM = R"_(
<tr><th>$loc$</th></tr>
<tr><td class="l">Temperature:</td><td>$t$ &#8451;</td></tr>
<tr><td class="l">Temperature (forecast):</td><td>$tf$ &#8451;</td></tr>
<tr><td class="l">Weather (forecast):</td><td>$df$</td></tr>
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
	logPrintfX(F("WG"), F("OWM Status..."));
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
				{F("tf"), String(w.temperatureForecast)},
				{F("df"), w.description},
		};
		macroStringReplace(owmStatusPageFS, mapLookup(m), ss);
	}

	logPrintfX(F("WG"), F("Sending footer..."));
	macroStringReplace(owmFooterPageFS, [](const char*) {return String();}, ss);
	logPrintfX(F("WG"), F("Total size: %d"), ss.buffer.length());

	webServer.send(200, textHtml, ss.buffer);
}

String WeatherGetter::getWeatherDescription()
{
	String r;
	r.reserve(256);

	for (size_t i = 0; i < weathers.size(); ++i)
	{
		const Weather& w = weathers[i];

		if (w.location.length() == 0)
			continue;

		r += w.location;
		r += " ";
		r += String(w.temperature, 1);
		r += '\x80';
		r += 'C';
		r += " (";
		r += String(w.temperatureForecast, 1);
		r += '\x80';
		r += 'C';
		r += ", ";
		r += w.description;
		r += ")";
		//		r += " ";
		//		r += w.pressure;
		//		r += " hPa";

		r += " -- ";
	}

	if (r.endsWith(" -- "))
	{
		r.remove(r.length() - 4);	//remove the end
	}

	return r;
}
