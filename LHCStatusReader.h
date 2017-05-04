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
#include "WiFiClient.h"
#include "ESPAsyncTCP.h"

class LHCStatusReader: public Tasks::TaskCRTP<LHCStatusReader>
{
	public:
		LHCStatusReader();

		void connect();
		void connected();
		void subscribe();

		//this is not a state any more
		void readData(uint8_t* data, size_t size);

		virtual void reset();

		virtual ~LHCStatusReader() = default;

	private:
		//WiFiClient connection;
		AsyncClient connection;
		CustomWebSocketPacketWrapper wspWrapper;
		AJSP::Parser jsonParser;
		PathListener jsonListener;
		uint32_t idlePackets = 0;
};

#endif /* LHCSTATUSREADER_H_ */
