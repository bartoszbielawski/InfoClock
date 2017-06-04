/*
 * LHCStatusReader.h
 *
 *  Created on: 11.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef LHCSTATUSREADER_H_
#define LHCSTATUSREADER_H_

#include "task.hpp"
#include "AJSP.hpp"
#include "CustomWebSocketPacketWrapper.h"
#include "PathListener.h"
#include "ESPAsyncTCP.h"
#include "MapCollector.hpp"

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

		void parseEnergy(const std::string&);
		void parsePage1Comment(const std::string&);
		void parseBeamMode(const std::string& value);

		virtual ~LHCStatusReader() = default;

		const String& getBeamMode() const {return beamMode;}
		const String& getPage1Comment() const {return page1Comment;}
		float 		  getBeamEnergy() const {return beamEnergy;}
		uint32_t      getPacketCount() const {return packetsRcvd;}

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
};

#endif /* LHCSTATUSREADER_H_ */
