/*
 * FixerIOReader.h
 *
 *  Created on: 14.11.2017
 *      Author: caladan
 */

#ifndef FIXERIOREADER_H_
#define FIXERIOREADER_H_

#include <Arduino.h>
#include <tasks.hpp>

class ESP8266WebServer;

class FixerIOReader: public Tasks::Task
{
	public:
		FixerIOReader();
		virtual ~FixerIOReader() = default;

		virtual void reset();
		virtual void run();

		String getRate();

		void handleConfig(ESP8266WebServer& webServer);

	private:
		String from;
		String to;
		float value = 0.0f;
};

#endif /* FIXERIOREADER_H_ */
