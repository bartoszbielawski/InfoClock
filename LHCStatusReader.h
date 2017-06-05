/*
 * LHCStatusReader.h
 *
 *  Created on: 11.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef LHCSTATUSREADER_H_
#define LHCSTATUSREADER_H_

#include "C-Tasks/task.hpp"
#include "AJSP/AJSP.hpp"
#include "CustomWebSocketPacketWrapper.h"
#include "ESPAsyncTCP.h"
#include "AJSP/MapCollector.hpp"

class ESP8266WebServer;

class LHCStatusReader: public Tasks::TaskCRTP<LHCStatusReader>
{
	public:
		LHCStatusReader();

		void connect();
		void connected();
		void subscribe();

		//this is not a state any more
		void readData(uint8_t* data, size_t size);
		//not a state really
		void parseData();

		void idle();

		virtual void reset();

		virtual ~LHCStatusReader() = default;

		void parseEnergy(const std::string&);
		void parsePage1Comment(const std::string&);
		void parseBeamMode(const std::string&);
	private:

		bool valid = false;
		float beamEnergy = 0.0f;
		String beamMode;
		String page1Comment;

 		/* connection variables */
		AsyncClient connection;
		CustomWebSocketPacketWrapper wspWrapper;
		MapCollector mc;

		/* connection stats */
		uint32_t packetsRcvd = 0;
		uint32_t idlePacketsRcvd = 0;
		uint32_t totalIdlePacketsRcvd = 0;
		uint32_t reconnects = 0;

		void handleStatusPage(ESP8266WebServer& ws);

		String getEnergy();
		String getStateInfo();
};

#endif /* LHCSTATUSREADER_H_ */
