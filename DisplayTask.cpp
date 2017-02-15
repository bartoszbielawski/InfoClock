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

DisplayTask::DisplayTask(uint32_t deviceCount): ledControl(LED_DTA, LED_CLK, LED_CS, deviceCount), scroll(ledControl)
{
	init();
}

void DisplayTask::init()
{
	index = displayStates.size()-1;
	nextDisplay();
}

void DisplayTask::reset()
{
	init();
}


void DisplayTask::pushMessage(const String& m, uint16_t sleep)
{
	DisplayState ds;
	ds.cycles = 1;
	currentMessage = m.c_str();
	ds.period = sleep;
	ds.scrolling = true;
	scroll.renderString(currentMessage.c_str(), myTestFont::font);
}

void DisplayTask::pushMessage(DisplayState ds)
{
	this->ds = ds;
	currentMessage = ds.fun();
	scroll.renderString(currentMessage.c_str(), myTestFont::font);
}

void DisplayTask::run()
{
	if (ds.scrolling)
	{
		if (scroll.tick())
		{
			nextDisplay();
		}

		sleep(ds.period);
		return;
	}

	if (--ds.cycles)
	{
		//refresh the message, render the new message
		currentMessage = ds.fun();
		scroll.renderString(currentMessage.c_str(), myTestFont::font);
		sleep(ds.period);
		return;
	}

	//regular display, take the next one and display
	nextDisplay();
	sleep(ds.period);
}

void DisplayTask::nextDisplay()
{
	do
	{
		index++;
		index %= displayStates.size();

		//returns by reference
		currentMessage = displayStates[index].fun();
	}
	while (currentMessage.length() == 0);
	ds = displayStates[index];

	logPrintf("Display: New message: %s", currentMessage.c_str());
	scroll.renderString(currentMessage, myTestFont::font);
}






