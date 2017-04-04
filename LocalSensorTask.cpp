/*
 * LocalSensorTask.cpp
 *
 *  Created on: 04.04.2017
 *      Author: caladan
 */

#include "LocalSensorTask.h"
#include "DataStore.h"
#include "utils.h"

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
	String s(t, 1);
	String p = "Ti:";
	s = p + s;
	s += "$C";
	DataStore::value("Local.Temperature") = s;
	logPrintf("LocalSensorTask: %s", s.c_str());
	dallasTemperature.requestTemperatures();
	sleep(5_s);
}

LocalSensorTask::~LocalSensorTask()
{

}

