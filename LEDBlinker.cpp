/*
 * LEDBlinker.cpp
 *
 *  Created on: 26.04.2017
 *      Author: caladan
 */

#include "LEDBlinker.h"
#include "Arduino.h"
#include "utils.h"
#include "config.h"
#include "tasks_utils.h"

LEDBlinker::LEDBlinker()
{
	pinMode(LBLUE, OUTPUT);
}

void LEDBlinker::run()
{
	digitalWrite(LBLUE, s);
	sleep(s ? 2_s - 1: 1);
	s = !s;
}

static RegisterTask r(new LEDBlinker);


