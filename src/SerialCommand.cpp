/*
 * SerialCommand.cpp
 *
 *  Created on: 30.04.2020
 *      Author: Bartosz Bielawski
 */

#include <SerialCommand.h>
#include <utils.h>
#include <DataStore.h>
#include <ESP8266WiFi.h>

SerialCommandTask::SerialCommandTask() {}

void SerialCommandTask::run()
{
    while (Serial.available())
    {
        auto c = Serial.read();
        if (c == '\r')
            continue;

        if (c != '\n')
        {
            cumulatedInput += (char)c;
            continue;
        }

        logPrintfX(F("SCT"), F("Command received: %s"), cumulatedInput.c_str());
    
        auto cmdAndParam = splitLine(cumulatedInput); 
        cumulatedInput.clear();

        const String& cmd = cmdAndParam.first;
        const String& param = cmdAndParam.second;
        const char* c_param = param.c_str();

        if (cmd == "reset")
        {
            rebootClock();
            continue;
        }

        if (cmd[0] == '$') 
        {
            String variableName = cmd.substring(1);
            const char* variableNameC = variableName.c_str();

             //read
            if (param.length() == 0)       
            {
                if (not DataStore::hasValue(variableName))
                {
                    logPrintfX(F("SCT"), F("Variable '%s' not found!"), variableNameC);
                    continue;
                }
                logPrintfX(F("SCT"), F("%s = '%s'"), variableNameC, dataSource(variableName).c_str());
                continue;
            }

            //write
            DataStore::value(variableName) = param;
            continue;
        }

        if (cmd == "variables")
        {
            for (const auto& k: DataStore::availableKeys())
            {
                logPrintfX(F("SCT"), F("%s = '%s'"), k.c_str(), DataStore::value(k).c_str());
            }
            continue;
        }

        if (cmd == "connected")
        {
            logPrintfX(F("SCT"), F("Connected = %s"), WiFi.isConnected() ? "true": "false");
            continue;
        }

        logPrintfX(F("SCT"), F("Unknown commmand!"));
        
    }
    sleep(0.1_s);
}

