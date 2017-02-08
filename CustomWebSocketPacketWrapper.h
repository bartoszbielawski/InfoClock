/*
 * WebSocketWrapper.h
 *
 *  Created on: 11.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef CUSTOMWEBSOCKETPACKETWRAPPER_H_
#define CUSTOMWEBSOCKETPACKETWRAPPER_H_

#include <cstdint>

class CustomWebSocketPacketWrapper
{
	public:
		CustomWebSocketPacketWrapper();

		enum class State: uint8_t
		{
			HEADER,
			LEN8,
			LEN16_HI,
			LEN16_LO,
			DATA_HEADER,
			DATA
		};


		State push(uint8_t c);

		virtual ~CustomWebSocketPacketWrapper();


		State 	 getState()  const {return state;}
		uint16_t getLength() const {return len;}

		void reset();

	protected:
		bool checkLength();

		State state = 		State::HEADER;
		uint16_t counter = 	0;
		uint16_t len = 0;
};

#endif /* CUSTOMWEBSOCKETPACKETWRAPPER_H_ */
