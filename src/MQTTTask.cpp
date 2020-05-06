/*
 * MQTTStuff.cpp
 *
 *  Created on: 22.03.2020
 *      Author: caladan
 */

#include <MQTTTask.h>
#include "utils.h"
#include "tasks_utils.h"
#include <MapCollector.hpp>
#include "WebServerTask.h"
#include "web_utils.h"
#include <DataStore.h>


MQTTTask::MQTTTask():
    mqttClient(wifiClient)
{
    addRegularMessage({this, [this](){return getMessage();}, 0.035_s, 1, true});

    mqttClient.setCallback([this](const char* topic, byte* payload, unsigned int length)
    {
        callback(topic, payload, length);
    });
    suspend();
}

void MQTTTask::reset()
{
    mqttClient.disconnect();
    auto mqttServer = DataStore::value(F("mqttServer"));

    if (mqttServer.length() == 0)
    {
        logPrintfX(F("MQT"), "Server not configured, suspending task...");
        suspend();
        return;
    }

    mqttClient.setServer(mqttServer.c_str(), 1883);

    auto clientId = DataStore::valueOrDefault(F("mqttClientId"), F("InfoClock"));
    auto user = DataStore::value(F("mqttUser"));
    auto passwd = DataStore::value(F("mqttPassword"));

    logPrintfX(F("MQT"), "Connecting (%s, %s)", mqttServer.c_str(), user.c_str());

    if (not mqttClient.connect(clientId.c_str(), user.c_str(), passwd.c_str()))
        return;

    logPrintfX(F("MQT"), "Connected!");

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/+", clientId.c_str());

    logPrintfX(F("MQT"), "Subscribing to %s...", topic);
    mqttClient.unsubscribe(topic);
    mqttClient.subscribe(topic);
    mqttClient.loop();
}

void MQTTTask::callback(const char* topic_raw, byte* payload, unsigned int length)
{
    char msg[128];
    memset(msg, 0, sizeof(msg));
    memcpy(msg, payload, length);
    String topic(topic_raw);
    logPrintfX(F("MQT"), "Msg with topic %s", topic_raw);

    if (topic.endsWith("push"))
    {
        String m = msg;
        DisplayTask::getInstance().pushMessage(m, 0.05_s, true);
        message = String();
        logPrintfX(F("MQT"), F("New push message: %s"), msg);
        return;
    }

    if (topic.endsWith("looped"))
    {
        message = msg;
        logPrintfX(F("MQT"), F("New looped message: %s"), msg);
        return;
    }

    if (topic.endsWith("request"))
    {
        String p(msg);
        if (p.endsWith("Password"))
            return;

        logPrintfX(F("MQT"), "Requested field: %s", p.c_str());

        char returnedTopic[128];
        auto clientId = DataStore::valueOrDefault(F("mqttClientId"), F("InfoClock"));
        snprintf(returnedTopic, sizeof(returnedTopic), "%s/publish/%s", clientId.c_str(), msg);

        auto variable = dataSource(p);
        logPrintfX(F("MQT"), F("Sending %s to %s"), variable.c_str(), returnedTopic);

        mqttClient.publish(returnedTopic, variable.c_str());

        mqttClient.loop();
    }
}

void MQTTTask::run()
{
    if (!mqttClient.connected())
    {
        reset();
        sleep(60_s);
        return;
    }

    if ((time(NULL) - lastReport) > 60)
    {
        auto clientId = DataStore::valueOrDefault(F("mqttClientId"), F("InfoClock"));
        String reports = DataStore::valueOrDefault(F("mqttReports"), String());
        if (reports.length())
        {
            auto varNames = tokenize(reports,",");
            for (const auto& s: varNames)
            {
                char topic[128];
                const char *sp = s.c_str(); 
                logPrintfX(F("MQT"), F("Reporting %s..."), sp);
                snprintf(topic, sizeof(topic), "%s/publish/%s", clientId.c_str(), sp);
                String value = dataSource(s);
                if (value.isEmpty())
                {
                    logPrintfX(F("MQT"), F("Variable %s not found!"), sp);
                    continue;
                }

                mqttClient.publish(topic, dataSource(s).c_str());
            }
        }
        lastReport = time(NULL);
    }

    mqttClient.loop();
    sleep(0.05_s);
}
