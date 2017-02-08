/*
 * WebSocketWrapper.cpp
 *
 *  Created on: 11.01.2017
 *      Author: Bartosz Bielawski
 */


#include "CustomWebSocketPacketWrapper.h"

#include "utils.h"

#include "Arduino.h"

CustomWebSocketPacketWrapper::CustomWebSocketPacketWrapper()
{
}

void CustomWebSocketPacketWrapper::reset()
{
	state = State::HEADER;
	counter = 0;
	len = 0;
}

bool CustomWebSocketPacketWrapper::checkLength()
{
	counter++;
	bool result = (counter == len);
	if (result) state = State::HEADER;
	return result;
}

CustomWebSocketPacketWrapper::State CustomWebSocketPacketWrapper::push(uint8_t c)
{
	State previousState = state;

	switch (state)
	{
		case State::HEADER:
			reset();
			if (c == 0x81) state = State::LEN8;
			break;

		case State::LEN8:
			if ((c & 0x7F) == 0x7E)
			{
				state = State::LEN16_HI;
				break;
			}

			len = c;		//if it's not 0x7E it's the short one
			state = State::DATA_HEADER;
			break;

		case State::LEN16_HI:
			len = c << 8;
			state = State::LEN16_LO;
			break;

		case State::LEN16_LO:
			len |= c;
			state = State::DATA_HEADER;
			break;

		case State::DATA_HEADER:
			checkLength();
			if (c == '|') state = State::DATA;
			break;

		case State::DATA:
			checkLength();
			break;
	}

	return previousState;
}

CustomWebSocketPacketWrapper::~CustomWebSocketPacketWrapper()
{
}

