/*
 * DisplayTask.cpp
 *
 *  Created on: 15.02.2017
 *      Author: caladan
 */


#include "DisplayTask.hpp"

#include "pyfont.h"
#include "myTestFont.h"
#include "LEDMatrixDriver.h"

#include "utils.h"
#include "DataStore.h"

#include "config.h"

static MessageProvider getFromDataStore(const String& name)
{
	//TODO: check: capture expression copies value or reference?
	return [name]() {return DataStore::valueOrDefault(name, String());};
}


static const std::vector<DisplayState> displayStates =
{
	{getTime, 										1_s,	10,	false},
	{getDate, 										2_s,	1,	false},
	{getFromDataStore(("webmessage")),				0.1_s,	1,	true},		//webmessage
	{getFromDataStore(("Local.Temperature")),		2_s,	1,	false},
	{getFromDataStore(("OWM.Temperature")),			2_s,	1,  false},
	{getFromDataStore(("OWM.Pressure")),			2_s,	1,	false},
	{getFromDataStore(("LHC.Page1Comment")),		0.1_s,	1,	true},
	{getFromDataStore(("LHC.BeamMode")),		  	0.1_s,	1, 	true},
	{getFromDataStore(("LHC.BeamEnergy")),			2_s, 1, false},
};

DisplayTask::DisplayTask(uint32_t deviceCount):
		TaskCRTP(&DisplayTask::nextMessage),
		ledMatrixDriver(deviceCount, LED_CS), scroll(ledMatrixDriver)
{
	init();
}

void DisplayTask::init()
{
	index = displayStates.size()-1;
}

void DisplayTask::reset()
{
	init();
}


void DisplayTask::pushMessage(const String& m, uint16_t sleep, bool scrolling)
{
	priorityMessages.push_back(DisplayState{[m](){return m;}, sleep, 1, scrolling});

	//wake up thread and interrupt the other display
	//only if it's not a priority message
	if (!priorityMessagePlayed)
	{
		nextState = &DisplayTask::nextMessage;
		resume();
	}
}

void DisplayTask::scrollMessage()
{
	bool done = scroll.tick();
	sleep(ds.period);

	if (done)
	{
		logPrintf("Scrolling done...");
		nextState = &DisplayTask::nextMessage;
	}
}

void DisplayTask::nextMessage()
{
	//load the next display
	nextDisplay();

	if (ds.scrolling)
	{
		const String& msg = ds.fun();
		scroll.renderString(msg, myTestFont::font);
		nextState = &DisplayTask::scrollMessage;
		return;
	}

	nextState = &DisplayTask::refreshMessage;
}


void DisplayTask::refreshMessage()
{
	scroll.renderString(ds.fun(), myTestFont::font);

	if (--ds.cycles == 0)
		nextState = &DisplayTask::nextMessage;

	sleep(ds.period);
}


void DisplayTask::nextDisplay()
{
	//special case for priority messages
	if (priorityMessages.size())
	{
		ds = priorityMessages.front();
		priorityMessages.erase(priorityMessages.begin());
		logPrintf("New message from PQ: %s", ds.fun().c_str());
		priorityMessagePlayed = true;
		return;
	}

	priorityMessagePlayed = false;

	//otherwise get back to the regular display
	do
	{
		index++;
		index %= displayStates.size();
	}
	while (displayStates[index].fun().length() == 0);

	ds = displayStates[index];
	logPrintf("New message from RQ: %s", ds.fun().c_str());
}
