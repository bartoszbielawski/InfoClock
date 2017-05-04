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
#include "config.h"

LocalSensorTask::LocalSensorTask():
	oneWire(ONE_WIRE_TEMP),
	dallasTemperature(&oneWire)
{
	//this is need
#ifdef OW_GND
	pinMode(OW_GND, OUTPUT);
	digitalWrite(OW_GND, 0);
#endif

	//initialize and request the temperature right away
	dallasTemperature.begin();
	dallasTemperature.requestTemperatures();
}

void LocalSensorTask::run()
{
	float t = dallasTemperature.getTempCByIndex(0);
	if (t == -127.0f)
	{
		logPrintfX(F("LST"), F("Sensor not found..."));
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
	logPrintfX(F("LST"), F("T = %s"), p.c_str());
	dallasTemperature.requestTemperatures();
	sleep(10_s);
}

LocalSensorTask::~LocalSensorTask()
{

}

static RegisterTask r(new LocalSensorTask, TaskDescriptor::SLOW);

