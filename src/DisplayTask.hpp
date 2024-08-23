/*
 * ledClock.hpp
 *
 *  Created on: 01.01.2017
 *      Author: Bartosz Bielawski
 */


#pragma once

#ifndef DISPLAYTASK_HPP_
#define DISPLAYTASK_HPP_

#include <tasks.hpp>
#include <LEDMatrixDriver.hpp>
#include "SDD.hpp"
#include "web_utils.h"
#include "config.h"

using MessageProvider = std::function<String()>;

struct DisplayState
{
		void*		owner;
		MessageProvider fun;
		
		uint16_t	period;
		uint16_t 	cycles;
		bool		scrolling;		//refresh till it's done		
};


class DisplayTask: public Tasks::TaskCRTP<DisplayTask>
{
	public:
		DisplayTask();

		void init();
		virtual void reset();

		void pushMessage(const String& m, uint16_t sleep, bool scrolling = false);

		void nextMessage();
		void scrollMessage();
		void refreshMessage();

		void addRegularMessage(const DisplayState& ds);
		void removeRegularMessages(void* owner);

		void addClock();

		static DisplayTask& getInstance();

	private:
		void nextDisplay();

		int index = 0;
		LEDMatrixDriver ledMatrixDriver;
		SDD scroll;
		DisplayState ds;

		std::vector<DisplayState> regularMessages;
		std::vector<DisplayState> priorityMessages;
		bool		priorityMessagePlayed = false;
		String 		currentMessage;

	public:
		void handleConfigPage(ESP8266WebServer& webServer);
};


#endif /* DISPLAYTASK_HPP_ */
