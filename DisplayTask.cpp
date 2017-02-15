/*
 * DisplayTask.cpp
 *
 *  Created on: 15.02.2017
 *      Author: caladan
 */


#include "DisplayTask.hpp"

#include "pyfont.h"
#include "myTestFont.h"
#include "LedControl.h"

#include "utils.h"
#include "DataStore.h"

#include "config.h"

static MessageProvider getFromDataStore(const String& name)
{
	//TODO: check: capture expression copies value or reference?
	return [name]() {return DataStore::value(name);};
}

static const std::vector<DisplayState> displayStates =
{
	{getTime, 										1_s,	10,	false},
	{getDate, 										2_s,	1,	false},
	{getFromDataStore("webmessage"),				0.3_s,	1,	true},		//webmessage
	{getFromDataStore("OWM.Temperature"),			2_s,	1,	false},
	{getFromDataStore("OWM.Pressure"),				2_s,	1,	false},
	{getFromDataStore("LHC.Page1Comment"),			0.3_s,	1,	true},
	{getFromDataStore("LHC.BeamMode"),			  	0.3_s,	1, 	true},
	{getFromDataStore("LHC.BeamEnergy"),			2_s, 1, false},
};

DisplayTask::DisplayTask(uint32_t deviceCount):
		TaskCRTP(&DisplayTask::nextMessage),
		ledControl(LED_DTA, LED_CLK, LED_CS, deviceCount), scroll(ledControl)
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


void DisplayTask::pushMessage(String m, uint16_t sleep, bool scrolling)
{

	DisplayState ds;

	ds.cycles = 1;
	ds.fun = [m](){return m;};
	ds.period = sleep;
	ds.scrolling = scrolling;

	priorityMessages.push_back(ds);

	//execute this one if there's only one message in the queue
	if (priorityMessages.size() == 1)
	{
		nextState = &DisplayTask::nextMessage;
		resume();		//resume the task so it can start running right away
	}
}

void DisplayTask::scrollMessage()
{
	logPrintf("Scrolling message...");
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
		scroll.renderString(ds.fun(), myTestFont::font);
		nextState = &DisplayTask::scrollMessage;
		return;
	}

	nextState = &DisplayTask::refreshMessage;
}


void DisplayTask::refreshMessage()
{
	logPrintf("Refreshing message...");
	ds.cycles--;
	scroll.renderString(ds.fun(), myTestFont::font);
	if (ds.cycles == 0)
		nextState = &DisplayTask::nextMessage;

	sleep(ds.period);
}

void DisplayTask::singleMessage()
{
	//maybe not needed as this is actually as refreshMessage
	scroll.renderString(ds.fun(), myTestFont::font);
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
		return;
	}

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
