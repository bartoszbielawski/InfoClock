/*
 * WeatherGetter.h
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef WEATHERGETTER_H_
#define WEATHERGETTER_H_

#include "task.hpp"
#include "ESP8266HTTPClient.h"
#include "AJSP.hpp"
#include "PathListener.h"

class WeatherGetter: public Tasks::Task
{
	public:
		WeatherGetter();
		virtual ~WeatherGetter() {}
		virtual void reset();
		virtual void run();
};

#endif /* WEATHERGETTER_H_ */
