/*
 * MQTTStuff.h
 *
 *  Created on: 22.03.2020
 *      Author: caladan
 */

#ifndef MQTTTASK_H
#define MQTTTASK_H

#include <tasks.hpp>
#include <PubSubClient.h>
#include <WiFiClient.h>

class MQTTTask: public Tasks::Task
{
	public:
		MQTTTask();
		virtual ~MQTTTask() = default;

		virtual void run();

		virtual void reset();

        String getMessage() const {return message;}

	private:
		WiFiClient wifiClient;
        PubSubClient mqttClient;
        String message;

		time_t lastReport = 0;
		
        void callback(const char* topic, byte* payload, unsigned int length);
};

#endif /* MQTTTASK_H */
