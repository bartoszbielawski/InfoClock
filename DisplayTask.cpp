/*
 * DisplayTask.cpp
 *
 *  Created on: 15.02.2017
 *      Author: Bartosz Bielawski
 */


#include "DisplayTask.hpp"

#include "pyfont.h"
#include "myTestFont.h"

#include "utils.h"
#include "tasks_utils.h"
#include "DataStore.h"

#include "config.h"

//static MessageProvider getFromDataStore(const String& name)
//{
//	//TODO: check: capture expression copies value or reference?
//	return [name]() {return DataStore::valueOrDefault(name, String());};
//}


DisplayTask::DisplayTask(uint32_t deviceCount):
		TaskCRTP(&DisplayTask::nextMessage),
		ledMatrixDriver(deviceCount, LED_CS), scroll(ledMatrixDriver),
		regularMessages({
			{this, getTime, 1_s,	10,	false},
			{this, getDate, 2_s,	1,	false},
			})
{
	init();
}


void DisplayTask::addRegularMessage(const DisplayState& ds)
{
	regularMessages.push_back(ds);
}

void DisplayTask::removeRegularMessages(void* owner)
{
	regularMessages.erase(
			std::remove_if(
					regularMessages.begin(),
					regularMessages.end(),
					[owner] (const DisplayState& ds) {return ds.owner == owner;}));
}

void DisplayTask::init()
{
	index = regularMessages.size()-1;
}

void DisplayTask::reset()
{
	init();
}


void DisplayTask::pushMessage(const String& m, uint16_t sleep, bool scrolling)
{
	priorityMessages.push_back(DisplayState{this, [m](){return m;}, sleep, 1, scrolling});

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
		logPrintfX(F("DT"), F("Scrolling done..."));
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
	//this flag will alow slow tasks to execute only if there is a two second sleep ahead
	slowTaskCanExecute = ds.period >= 1_s;
	//logPrintfX(F("DT"), F("slowTask: %s"), slowTaskCanExecute ? "true": "false");
}


void DisplayTask::nextDisplay()
{
	//special case for priority messages
	if (priorityMessages.size())
	{
		ds = priorityMessages.front();
		priorityMessages.erase(priorityMessages.begin());
		logPrintfX(F("DT"), F("New message from PQ = %s"), ds.fun().c_str());
		priorityMessagePlayed = true;
		return;
	}

	priorityMessagePlayed = false;

	//otherwise get back to the regular display
	do
	{
		index++;
		index %= regularMessages.size();
	}
	while (regularMessages[index].fun().length() == 0);

	ds = regularMessages[index];
	logPrintfX(F("DT"), F("New message from RQ = %s"), ds.fun().c_str());
}
