/*
 * FixerIOReader.cpp
 *
 *  Created on: 14.11.2017
 *      Author: caladan
 */

#include "FixerIOReader.h"

#include "Arduino.h"
#include "pgmspace.h"
#include "ESP8266HTTPClient.h"
#include "utils.h"
#include "tasks_utils.h"
#include "web_utils.h"

static const char fixerIOPageUrl[] PROGMEM = "http://api.fixer.io/latest?base=%s&symbols=%s";

FixerIOReader::FixerIOReader()
{
	from = readConfig(F("fixerIOFrom"));
	if (from.length() == 0)
		from = "CHF";
	to = readConfig(F("fixerIOTo"));
	if (to.length() == 0)
		to = "EUR";

	getDisplayTask().addRegularMessage({this, [this](){return getRate();}, 0.08_s, 1, true});
	getDisplayTask().addClock();

	getWebServerTask().registerPage(F("fio"), F("Fixer.io"),
			[this]  (ESP8266WebServer& ws) mutable {handleConfig(ws);});
}

String FixerIOReader::getRate()
{
	if (value == 0.0f)
		return String();

	String r = from;
	r += "->";
	r += to;
	r += ": ";
	String v(value, 3);
	r += v;
	return r;
}

void FixerIOReader::run()
{
	HTTPClient httpClient;

	logPrintfX(F("FIOR"), F("Reading conversion rate status"));

	char url[128];

	snprintf_P(url, sizeof(url), fixerIOPageUrl, from.c_str(), to.c_str());

	logPrintfX(F("FIOR"), url);

	httpClient.begin(url);

	int httpCode = httpClient.GET();
	if (httpCode != 200)
	{
		logPrintfX(F("FIOR"), F("HTTP code: %d"), httpCode);
		reset();		//this resets variables
		sleep(3600_s);
		return;
	}

	auto httpStream = httpClient.getStream();
	snprintf_P(url, sizeof(url), R"("%s":)", to.c_str());

	bool found = httpStream.findUntil(url,"}}");
	if (!found)
	{
		value = 0.0f;
		sleep(3600_s);
		return;
	}

	value = httpStream.parseFloat();

	String v(value, 3);

	logPrintfX(F("FIOR"), F("%s->%s: %s"), from.c_str(), to.c_str(), v.c_str());
	sleep(3600_s);
}

void FixerIOReader::reset()
{

}

static const char fioConfigPage[] PROGMEM = R"_(
<form method="post" action="fio" autocomplete="on">
<table>
<tr><th>Exchange rates</th></tr>
<tr><td class="l">$from$ &rarr; $to$</td><td>$value$</td></tr>
<tr><th>Configuration</th></tr>
<tr><td class="l">From:</td><td><input type="text" name="from" value="$from$"></td></tr>
<tr><td class="l">To:</td><td><input type="text" name="to" value="$to$"></td></tr>
<tr><td/><td><input type="submit"></td></tr>
</table></form></body></html>
)_";

FlashStream fioConfigPageFS(fioConfigPage);

void FixerIOReader::handleConfig(ESP8266WebServer& webServer)
{
	if (!handleAuth(webServer)) return;

	auto from = webServer.arg(F("from"));
	auto to   = webServer.arg(F("to"));

	if (from.length()) 	writeConfig(F("fixerIOFrom"), from);
	if (to.length()) 	writeConfig(F("fixerIOTo"),   to);

	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("Fixer.io"), ss);

	std::map<String, String> m = {
			{F("from"), 		readConfig(F("fixerIOFrom"))},
			{F("to"), 			readConfig(F("fixerIOTo"))},
			{F("value"),		String(value, 3)},
	};

	macroStringReplace(fioConfigPageFS, mapLookup(m), ss);
	webServer.send(200, textHtml, ss.buffer);

	//refresh the data
	resume();
}


static RegisterTask fiot(new FixerIOReader, TaskDescriptor::CONNECTED | TaskDescriptor::SLOW);


