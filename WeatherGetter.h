/*
 * WeatherGetter.h
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef WEATHERGETTER_H_
#define WEATHERGETTER_H_

#include <task.hpp>
#include "ESP8266HTTPClient.h"
#include "CounterCRTP.hpp"
#include <vector>

class ESP8266WebServer;

class WeatherGetter: public Tasks::Task, public CounterCRTP<WeatherGetter>
{
	public:
		WeatherGetter();
		virtual ~WeatherGetter() {}
		virtual void reset();
		virtual void run();
	private:
		struct Weather
		{
			Weather(uint32_t locationId):
				locationId(locationId),
				pressure(0),
				temperature(0) {}

			uint32_t locationId;
			int16_t pressure;
			float  temperature;
			String location;
		};

		std::vector<Weather> weathers;

		uint32_t currentWeatherIndex;

		String apiKey;

		//page handling
		void handleConfig(ESP8266WebServer& ws);
		void handleStatus(ESP8266WebServer& ws);

		//display function
		String getWeatherDescription(uint32_t index);
};

#endif /* WEATHERGETTER_H_ */
