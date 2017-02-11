/*
 * ledClock.hpp
 *
 *  Created on: 01.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef LEDCLOCK_HPP_
#define LEDCLOCK_HPP_

#include "task.hpp"
#include "pyfont.h"
#include "myTestFont.h"
#include "LedControl.h"
#include "SDD.hpp"

#include "DataStore.h"



using MessageProvider = std::function<String()>;

static MessageProvider getFromDataStore(const String& name)
{
	return [name]() {return dataStore().value(name);};
}

struct DisplayState
{
		MessageProvider fun;
		uint16_t	period;
		uint16_t 	cycles;
		bool		scrolling;		//refresh till it's done
};

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

class LedClock: public Tasks::Task
{
	public:
		LedClock(uint32_t deviceCount): ledControl(LED_DTA, LED_CLK, LED_CS, deviceCount), scroll(ledControl)
		{
			init();
		}

		void init()
		{
			index = displayStates.size()-1;
			nextDisplay();
		}

		virtual void reset()
		{
			init();
		}


		void pushMessage(const String& m, uint16_t sleep)
		{
			DisplayState ds;
			ds.cycles = 1;
			currentMessage = m.c_str();
			ds.period = sleep;
			ds.scrolling = true;
			scroll.renderString(currentMessage.c_str(), myTestFont::font);
		}

		void pushMessage(DisplayState ds)
		{
			this->ds = ds;
			currentMessage = ds.fun();
			scroll.renderString(currentMessage.c_str(), myTestFont::font);
		}

		void run()
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

	private:
		void nextDisplay()
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

		int index = 0;
		LedControl ledControl;
		SDD scroll;
		String currentMessage;
		DisplayState ds;
};




#endif /* LEDCLOCK_HPP_ */
