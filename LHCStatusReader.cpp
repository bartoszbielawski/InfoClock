/*
 * LHCStatusReader.cpp
 *
 *  Created on: 11.01.2017
 *      Author: Bartosz Bielawski
 */


#include "Arduino.h"
#include "pgmspace.h"

#include "LHCStatusReader.h"
#include "DataStore.h"
#include "utils.h"
#include "tasks_utils.h"
#include "web_utils.h"

//address and port of the broadcast server
static const char hostname[] = "prod-bcast-ws-01.cern.ch";
static const int16_t port  = 8080;

//GET request (three parts interleaved with UUIDs)
static const char httpGetRequestStart[] PROGMEM =   "GET /broadcast/atmo/broadcast/";		//UUID here
static const char httpGetRequestMiddle[] PROGMEM = "?X-Atmosphere-tracking-id=";			//UUID here
static const char httpGetRequestEnd[] PROGMEM =
		"&X-Atmosphere-Framework=2.2.11-javascript&X-Atmosphere-Transport=websocket&X-Atmosphere-TrackMessageSize=true&X-atmo-protocol=true HTTP/1.1\r\n"
		"Host: prod-bcast-ws-01.cern.ch:8080\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"Sec-WebSocket-Key: f0Kjabu532ly+dpVOieU3g==\r\n"
		"Connection: keep-alive, Upgrade\r\n"
		"Pragma: no-cache\r\n"
		"Cache-Control: no-cache\r\n"
		"Upgrade: websocket\r\n\r\n";

//subscription string
static const char subscriptionRequest[] PROGMEM = R"_({"listOfSubscriptionsToAdd":["dip://dip/acc/LHC/RunControl/BeamMode","dip://dip/acc/LHC/RunControl/Page1","dip://dip/acc/LHC/Beam/Energy"]})_";

/*
 * BEAM ENERGY
 */

void LHCStatusReader::parseEnergy(const std::string& value)
{
	//0.12 GeV per LSB
	float newBeamEnergy = atoi(value.c_str())+1;
	newBeamEnergy *= 0.120;		//in GeV

	//don't update, don't print it...
	if (newBeamEnergy == beamEnergy)
		return;

	beamEnergy = newBeamEnergy;

	if ((beamEnergy < 50) or (beamEnergy > 7800)) 		//450 GeV - nominal energy at injection
	{
		beamEnergy = 0.0f;
		return;
	}

	String beamEnergys(beamEnergy, 0);
	beamEnergys += F(" GeV");
	logPrintfA(F("LSR"), F("E = %s"), beamEnergys.c_str());
}

/*
 * PAGE 1 COMMENT
 */

void LHCStatusReader::parsePage1Comment(const std::string& value)
{
	page1Comment = value.c_str();
	page1Comment.trim();
	page1Comment.replace(F("\n\n"), F(" -- "));
	page1Comment.replace('\n', ' ');

	logPrintfA(F("LSR"), F("P1 Comment = %s"), page1Comment.c_str());
}

/*
 * BEAM MODE
 */

void LHCStatusReader::parseBeamMode(const std::string& value)
{
	beamMode = value.c_str();
	logPrintfA(F("LSR"), F("BM = %s"), value.c_str());
}



bool LSRPathFilter(const std::string& path_, const std::string& value_)
{
	String path(path_.c_str());

	bool isPublicationName = path.endsWith(F("publicationName"));
	if (isPublicationName)
		return true;

	String value(value_.c_str());
	return path.endsWith(F("value"));
}

LHCStatusReader::LHCStatusReader():
		TaskCRTP<LHCStatusReader>(&LHCStatusReader::connect),
		mc(LSRPathFilter)
{
	connection.setRxTimeout(30);	//no RX data for 30 seconds
	sleep(15_s);

	getWebServerTask().registerPage("lhc", "LHC Status",
			[this](ESP8266WebServer& ws) {handleStatusPage(ws);});

	getDisplayTask().addRegularMessage({this, [this](){return getEnergy();}, 2_s, 1, false});
	getDisplayTask().addRegularMessage({this, [this](){return getStateInfo();}, 0.025_s, 1, true});
}

void LHCStatusReader::reset()
{
	//reset variables
	valid = false;
	page1Comment = "";
	beamEnergy = 0.0f;
	beamMode = "";

	logPrintfX(F("LSR"), F("Reset"));

	connection.abort();
	if (connection.connected())
		connection.close();

	wspWrapper.reset();
	mc.reset();

	idlePacketsRcvd = 0;
	packetsRcvd = 0;

	nextState = &LHCStatusReader::connect;
	sleep(5_s);
}

static void discardData(void*, AsyncClient*, void*, size_t)
{
	//do nothing
}

void LHCStatusReader::connect()
{
	logPrintfX(F("LSR"), F("Connecting to the LHC status server"));

	connection.onConnect([] (void* o, AsyncClient*) {
		static_cast<LHCStatusReader*>(o)->connected();
	}, this);

	/* SPECIAL CASE HANDLING */
	connection.onDisconnect([] (void* o, AsyncClient*) {
		static_cast<LHCStatusReader*>(o)->reset();
	}, this);

	connection.onTimeout([] (void* o, AsyncClient*, uint32) {
		static_cast<LHCStatusReader*>(o)->reset();
	}, this);

	reconnects++;
	connection.connect(hostname, port);
	sleep(60_s);
}

void LHCStatusReader::connected()
{
	if (!connection.connected())
	{
		reset();
		return;		//try again...
	}

	logPrintfA(F("LSR"), F("Connected"));

	if (!connection.canSend())
	{
		sleep(0.1_s);
		return;
	}

	//if we get anything just throw it away :)
	connection.onData(&discardData, this);

	//TODO: local copy using a local buffer, not the String
	String s(httpGetRequestStart);
	connection.write(s.c_str(), s.length());
	connection.write(generateRandomUUID());
	s = httpGetRequestMiddle;
	connection.write(s.c_str(), s.length());
	connection.write(generateRandomUUID());
	s = httpGetRequestEnd;
	connection.write(s.c_str(), s.length());

	nextState = &LHCStatusReader::subscribe;

	sleep(1_s);		//wait for the response to arrive and discard it :)
}


void LHCStatusReader::subscribe()
{
	if (!connection.connected())
	{
		reset();
		return;
	}

	logPrintfX(F("LSR"), F("Subscribing..."));

	uint32_t rnd = os_random();
	uint8_t k[] = {(rnd >> 24) & 0xFF, (rnd >> 16) & 0xFF, (rnd >> 8) & 0xFF, rnd & 0xFF};

	//now we want to do something with the data, don't discard it any more
	connection.onData([](void* o, AsyncClient*, void* data, size_t size){
		static_cast<LHCStatusReader*>(o)->readData((uint8_t*)data, size);
	}, this);

	sendWSPacket_P(0x81, sizeof(subscriptionRequest), k, subscriptionRequest, &connection);


	nextState = &LHCStatusReader::idle;
}

void LHCStatusReader::idle()
{
	logPrintfX(F("LSR"), "Packets: %d RC: %d", packetsRcvd, reconnects);
	sleep(60_s);
}

void LHCStatusReader::readData(uint8_t* data, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		uint8_t c = data[i];
		auto state = wspWrapper.push(c);

		bool data = state == CustomWebSocketPacketWrapper::State::DATA;
		if (data)
		{
			if (wspWrapper.getLength() > 5)
			{
				if (mc.parse(c) == AJSP::Parser::Result::DONE)
				{
					packetsRcvd++;
					parseData();
				}
			}
			else
			{
				idlePacketsRcvd++;
				totalIdlePacketsRcvd++;
				if (idlePacketsRcvd > 5)
				{
					logPrintfA(F("LSR"), F("Idle message rcvd, restarting..."));
					reset();	//we have started receiving these short messages, restart
				}
				return;
			}

		}
	}
}

static const char beamEnergyDip[] PROGMEM = "dip://dip/acc/LHC/Beam/Energy";
static const char beamModeDip[] PROGMEM = 	"dip://dip/acc/LHC/RunControl/BeamMode";
static const char page1Dip[] PROGMEM = 		"dip://dip/acc/LHC/RunControl/Page1";

struct SubDesc
{
		PGM_P 		dipAddress;
		uint8_t 	newValueElement;
		void 		(LHCStatusReader::*func)(const std::string& value);
};


static const SubDesc sds[] =
{
		{beamEnergyDip, 1, &LHCStatusReader::parseEnergy},
		{beamModeDip, 3, &LHCStatusReader::parseBeamMode},
		{page1Dip, 3, &LHCStatusReader::parsePage1Comment}
};


void LHCStatusReader::parseData()
{
	const auto& v = mc.getValues();

	String key;
	String value;

	for (const auto& e: v)
	{
		value = e.second.c_str();
		for (const auto& sd: sds)
		{
			String da(sd.dipAddress);
			if (value.equals(da))
			{
				key = e.first.c_str();
				key.replace(F("publicationName"), F("newValue/"));
				key += String(sd.newValueElement);
				key += String(F("/value"));
				const auto j = v.find(key.c_str());
				if (j != v.end())
				{
					(this->*sd.func)(j->second);
				}
			}
		}
	}
	valid = true;
	mc.reset();
}


//------------- WEBPAGE STUFF


static const char lhcStatusPage[] PROGMEM = R"_(
<table>
<tr><th>LHC Status</th></tr>
<tr><td class="l">Beam mode:</td><td>$mode$</td></tr>
<tr><td class="l">Page 1 Comment:</td><td>$comment$</td></tr>
<tr><td class="l">Energy:</td><td>$energy$ GeV</td></tr>
<tr><td class="l">Packets:</td><td>$packets$</td></tr>
</table>
</body>
<script>setTimeout(function(){window.location.reload(1);}, 15000);</script>
</html>
)_";

FlashStream lhcStatusPageFS(lhcStatusPage);

void LHCStatusReader::handleStatusPage(ESP8266WebServer& webServer)
{
	StringStream ss(2048);
	macroStringReplace(pageHeaderFS, constString("LHC Status"), ss);

	std::map<String, String> m =
	{
			{F("mode"), beamMode},
			{F("comment"), page1Comment},
			{F("energy"), String(beamEnergy, 0)},
			{F("packets"), String(packetsRcvd)}
	};

	macroStringReplace(lhcStatusPageFS, mapLookup(m), ss);
	webServer.send(200, textHtml, ss.buffer);
}

// ----------------- DISPLAY STUFF -------------------

String LHCStatusReader::getStateInfo()
{
	if (beamMode.length() && page1Comment.length())
		return beamMode + ": " + page1Comment;

	return String();
}

String LHCStatusReader::getEnergy()
{
	if (beamEnergy == 0)
		return String();

	return String(beamEnergy, 0) + " GeV";
}

static RegisterTask rt1(new LHCStatusReader, TaskDescriptor::CONNECTED | TaskDescriptor::SLOW);

