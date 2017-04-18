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

//this path is reconfigured to match the currently processed element
//X is replaced with the current update, Y is replaced with the item inside newValue element
static char valuePath[] = "/root/updates/X/newValue/Y/value";

//replace the X and Y with the two values
void configurePath(char update, char value)
{
	valuePath[14] = update;
	valuePath[25] = value;
}

static const char beamEnergyDip[] = "dip://dip/acc/LHC/Beam/Energy";
static const char beamModeDip[] = 	"dip://dip/acc/LHC/RunControl/BeamMode";
static const char page1Dip[] = 		"dip://dip/acc/LHC/RunControl/Page1";

const char* lastKey = NULL;

/*
 * BEAM ENERGY
 */

void parseEnergy(const std::string& value)
{
	//0.12 GeV per LSB

	float energy = atoi(value.c_str());
	energy *= 0.120;		//in GeV

	if ((energy < 300) or (energy > 7800)) 		//450 GeV - nominal energy at injection
	{
		//65535 reported by the DIP service in some cases
		DataStore::value(F("LHC.BeamEnergy")) = String();
		return;
	}

	String beamEnergy(energy, 0);
	beamEnergy += F(" GeV");

	DataStore::value(F("LHC.BeamEnergy")) = beamEnergy;

	logPrintf("LHC E: %s", beamEnergy.c_str());
}

/*
 * PAGE 1 COMMENT
 */

void parsePage1Comment(const std::string& value)
{
	String page1Comment = value.c_str();
	page1Comment.trim();
	page1Comment.replace('\n', '-');

	DataStore::value(F("LHC.Page1Comment")) = page1Comment.c_str();

	logPrintf("LHC P1: %s", page1Comment.c_str());
}

/*
 * BEAM MODE
 */

void parseBeamMode(const std::string& value)
{
	DataStore::value(F("LHC.BeamMode")) = value.c_str();
	logPrintf("LHC BM: %s", value.c_str());
}

struct SubDesc
{
		const char* dipAddress;
		uint8_t 	newValueElement;
		void 		(*fun)(const std::string& value);
};


static const SubDesc sds[] =
{
		{beamEnergyDip, '1', parseEnergy},
		{beamModeDip, '3', parseBeamMode},
		{page1Dip, '3', parsePage1Comment}
};

//this function is called when the JSON parser meets a leaf element
static void onListenerMarch(const std::string& key, const std::string& value)
{
	//Serial.printf("K: %s V: %s\n", key.c_str(), value.c_str());
	//first check values from descriptors and if it matches - set the new last element
	//to match the update item and newValue item index
	for (const auto& sd: sds)
	{
		if (value == sd.dipAddress)
		{
			lastKey = sd.dipAddress;
			configurePath(key[14], sd.newValueElement);
			return;
		}
	}

	for (const auto& sd: sds)
	{
		if (lastKey == sd.dipAddress)
		{
			sd.fun(value);
			return;
		}
	}
}


LHCStatusReader::LHCStatusReader():
TaskCRTP<LHCStatusReader>(&LHCStatusReader::connect),
jsonListener(&jsonParser)
{
	jsonListener.monitoredPaths().push_back("/root/updates/0/publicationName");
	jsonListener.monitoredPaths().push_back("/root/updates/1/publicationName");
	jsonListener.monitoredPaths().push_back("/root/updates/2/publicationName");
	jsonListener.monitoredPaths().push_back(valuePath);
	jsonListener.setCallback(onListenerMarch);
	jsonParser.setListener(&jsonListener);
}

void LHCStatusReader::reset()
{
	//reset variables
	DataStore::value(F("LHC.Page1Comment")) = String();
	DataStore::value(F("LHC.BeamMode")) = String();
	DataStore::value(F("LHC.BeamEnergy")) = String();

	logPrintf(F("LHCStatusReader - Reset"));

	connection.stop();
	jsonParser.reset();
	wspWrapper.reset();

	nextState = &LHCStatusReader::connect;
	sleep(5_s);
}

void LHCStatusReader::connect()
{
	logPrintf(F("LHCStatusReader - connecting to the LHC status server..."));

	connection.setTimeout(1000);
	connection.connect(hostname, port);
	if (!connection.connected())
	{
		reset();
		return;		//try again...
	}

	logPrintf(F("LHCStatusReader - connected..."));

	connection.write_P(httpGetRequestStart, sizeof(httpGetRequestStart)-1);
	connection.write(generateRandomUUID());
	connection.write_P(httpGetRequestMiddle, sizeof(httpGetRequestMiddle)-1);
	connection.write(generateRandomUUID());
	connection.write_P(httpGetRequestEnd, sizeof(httpGetRequestEnd)-1);

	nextState = &LHCStatusReader::subscribe;
	sleep(0.1_s);		//wait for the response to arrive
}


void LHCStatusReader::subscribe()
{
	if (!connection.connected())
	{
		nextState = &LHCStatusReader::connect;
		sleep(5_s);
		return;
	}

	//flush
	while (int a = connection.available())
		connection.read();

	logPrintf(F("LHCStatusReader - subscribing..."));

	uint32_t rnd = os_random();
	uint8_t k[] = {(rnd >> 24) & 0xFF, (rnd >> 16) & 0xFF, (rnd >> 8) & 0xFF, rnd & 0xFF};


	sendWSPacket_P(0x81, sizeof(subscriptionRequest), k, subscriptionRequest, &connection);

	nextState = &LHCStatusReader::readData;
}

void LHCStatusReader::readData()
{
	if (!connection.connected())
	{
		nextState = &LHCStatusReader::connect;
		sleep(1_s);
		return;
	}

	while (connection.available())
	{
		uint8_t localBuffer[64];
		uint32_t readSize = std::min(connection.available(), (int)sizeof(localBuffer));

		readSize = connection.read(localBuffer, readSize);

		for (int i = 0; i < readSize; i++)
		{
			uint8_t c = localBuffer[i];
			auto state = wspWrapper.push(c);

//			if (state == CustomWebSocketPacketWrapper::State::DATA_HEADER)
//			{
//				logPrintf(F("PL: %d"), wspWrapper.getLength());
//			}

			bool data = state == CustomWebSocketPacketWrapper::State::DATA;
			if (data)
			{
				if (wspWrapper.getLength() > 5)
					jsonParser.parse(c);
				else
				{
					logPrintf(F("Received idle message from the service, restarting..."));
					reset();	//we have started receiving these short messages, restart
					return;
				}

			}

		}

		//reset the WDT after each batch of data
		wdt_reset();
	}

	sleep(0.1_s);
}



LHCStatusReader::~LHCStatusReader()
{
}

