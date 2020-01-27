/*
 * LHCStatusReaderNew.cpp
 *
 *  Created on: 19.10.2017
 *      Author: Bartosz Bielawski
 */


#include "Arduino.h"
#include "pgmspace.h"

#include "LHCStatusReaderNew.h"

#include "ESP8266HTTPClient.h"
#include "utils.h"
#include "tasks_utils.h"
#include "web_utils.h"

static const char pageUrl[] PROGMEM = "http://alicedcs.web.cern.ch/AliceDCS/monitoring/screenshots/rss.xml";

LHCStatusReaderNew::LHCStatusReaderNew()
{
	getWebServerTask().registerPage("lhc", "LHC Status",
			[this](ESP8266WebServer& ws) {handleStatusPage(ws);});

	getDisplayTask().addRegularMessage({this, [this](){return getStateInfo();}, 0.025_s, 1, true});
	getDisplayTask().addRegularMessage({this, [this](){return getEnergy();}, 2_s, 1, false});
	getDisplayTask().addClock();
	sleep(15_s);
}

void LHCStatusReaderNew::reset()
{
	sleep(15_s);
	page1Comment = String();
	beamEnergy = String();
	beamMode = String();
	refreshTime = String();
}


void LHCStatusReaderNew::run()
{
	HTTPClient httpClient;
	WiFiClient wifiClient;

	logPrintfX(F("LSRX"), F("Reading LHC Status"));
	httpClient.begin(wifiClient, pageUrl);

	int httpCode = httpClient.GET();
	if (httpCode != 200)
	{
		logPrintfX(F("LSRX"), F("HTTP code: %d"), httpCode);
		reset();		//this resets variables
		sleep(60_s);
		return;
	}

	auto httpStream = httpClient.getStream();

	while (httpStream.available())
	{
		bool found = httpStream.findUntil("<title>","</rss>");
		if (not found)
			break;

		auto title = httpStream.readStringUntil(':');
		if (title == F("LhcPage1"))
		{
			page1Comment = httpStream.readStringUntil('<');
			page1Comment.trim();
			page1Comment.replace(F("\n\n"), F(" -- "));
			page1Comment.replace('\n', ' ');
			logPrintfX(F("LHCX"), F("Page1Comment: %s"), page1Comment.c_str());
		}

		if (title == F("BeamEnergy"))
		{
			beamEnergy = httpStream.readStringUntil('<');
			beamEnergy.trim();
			logPrintfX(F("LHCX"), F("BeamEnergy: %s"), beamEnergy.c_str());
		}

		if (title == F("LhcBeamMode"))
		{
			beamMode = httpStream.readStringUntil('<');
			beamMode.trim();
			logPrintfX(F("LHCX"), F("BeamMode: %s"), beamMode.c_str());
		}
	}

	refreshTime = getDateTime();

	logPrintfX(F("LSRX"), F("Done!"));
	sleep(60_s);
}



//------------- WEBPAGE STUFF


static const char lhcStatusPage[] PROGMEM = R"_(
<table>
<tr><th>LHC Status</th></tr>
<tr><td class="l">Last refresh:</td><td>$timestamp$</td></tr>
<tr><td class="l">Beam mode:</td><td>$mode$</td></tr>
<tr><td class="l">Page 1 Comment:</td><td>$comment$</td></tr>
<tr><td class="l">Energy:</td><td>$energy$</td></tr>
</table>
</body>
<script>setTimeout(function(){window.location.reload(1);}, 15000);</script>
</html>
)_";

FlashStream lhcStatusPageNewFS(lhcStatusPage);

void LHCStatusReaderNew::handleStatusPage(ESP8266WebServer& webServer)
{
	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("LHC Status"), ss);

	std::map<String, String> m =
	{
			{F("timestamp"), refreshTime},
			{F("mode"), beamMode},
			{F("comment"), page1Comment},
			{F("energy"), beamEnergy}
	};

	macroStringReplace(lhcStatusPageNewFS, mapLookup(m), ss);
	webServer.send(200, textHtml, ss.buffer);
}

// ----------------- DISPLAY STUFF -------------------

String LHCStatusReaderNew::getStateInfo()
{
	if (beamMode.length() && page1Comment.length())
		return beamMode + ": " + page1Comment;

	return String();
}

String LHCStatusReaderNew::getEnergy()
{
	//ALICE reports some strange value during the shutdown
	if (beamEnergy.toInt() > 7100) return String();
	return beamEnergy;
}

//static RegisterTask rt1(new LHCStatusReaderNew, TaskDescriptor::CONNECTED | TaskDescriptor::SLOW);

