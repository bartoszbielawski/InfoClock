/*
 * LocalSensorTask.cpp
 *
 *  Created on: 04.04.2017
 *      Author: caladan
 */

#include "LocalSensorTask.h"
#include "DataStore.h"
#include "utils.h"
#include "tasks_utils.h"

LocalSensorTask::LocalSensorTask():
	oneWire(D3),
	dallasTemperature(&oneWire)
{
	pinMode(D2, OUTPUT);
	digitalWrite(D2, 0);
	//initialize and request the temperature right away
	dallasTemperature.begin();
	dallasTemperature.requestTemperatures();
}

void LocalSensorTask::run()
{
	float t = dallasTemperature.getTempCByIndex(0);
	if (t == -127.0f)
	{
		logPrintf(F("LST: Sensor not found..."));
		dallasTemperature.requestTemperatures();
		sleep(10_s);
		return;
	}

	String s(t, 1);
	String p = "\x81 ";
	p += s;
	p += (char)0x80;
	p += 'C';
	DataStore::value(F("Local.Temperature")) = p;
	logPrintf(F("LST: %s"), p.c_str());
	dallasTemperature.requestTemperatures();
	sleep(10_s);
}

LocalSensorTask::~LocalSensorTask()
{

}

static RegisterTask r(new LocalSensorTask);

