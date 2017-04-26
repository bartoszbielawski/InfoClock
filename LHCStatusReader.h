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

class LHCStatusReader: public Tasks::TaskCRTP<LHCStatusReader>
{
	public:
		LHCStatusReader();

		void connect();
		void subscribe();
		void readData();

		virtual void reset();

		virtual ~LHCStatusReader() = default;

	private:
		WiFiClient connection;
		CustomWebSocketPacketWrapper wspWrapper;
		AJSP::Parser jsonParser;
		PathListener jsonListener;
};

#endif /* LHCSTATUSREADER_H_ */
