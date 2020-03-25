/*
 * MQTTStuff.cpp
 *
 *  Created on: 22.03.2020
 *      Author: caladan
 */

#include <MQTTStuff.h>
#include "utils.h"
#include "tasks_utils.h"
#include <MapCollector.hpp>
#include "WebServerTask.h"
#include "web_utils.h"
#include <exception>
#include <DisplayTask.hpp>

MQTTTask::MQTTTask():
    mqttClient(wifiClient)
{
    getDisplayTask().addRegularMessage({
		this,
		[this](){return getMessage();},
		0.035_s,
		1,
		true});

    mqttClient.setServer("rpi.lan", 1883);
    mqttClient.setCallback([this](const char* topic, byte* payload, unsigned int length)
    {
        callback(topic, payload, length);
    });
    suspend();
}

void MQTTTask::callback(const char* topic, byte* payload, unsigned int length)
{
    logPrintfX("MQT", "%s", topic);
    char msg[128];
    memset(msg, 0, sizeof(msg));
    memcpy(msg, payload, length);
    String Topic(topic);
    if (Topic.endsWith("push"))
    {
        String m = msg;
        getDisplayTask().pushMessage(m, 0.05_s, true);
        message = String();
    }
    else
    {
        message = msg;
    }
}

void MQTTTask::run()
{
    if (!mqttClient.connected())
    {
        logPrintfX("MQT", "Connecting...");
        auto code = mqttClient.connect("ID", "mqtt", "mqtt");
        logPrintfX("MQT", "%d", code);
        mqttClient.subscribe("clockTopic/+");
        sleep(1.0_s);
        return;
    }   
    mqttClient.loop();
    sleep(1_s);
    logPrintfX("MQT", "Msg: %s", message.c_str());
}

//static RegisterTask rmqtt(new MQTTTask, TaskDescriptor::CONNECTED | TaskDescriptor::SLOW);