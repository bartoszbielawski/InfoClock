/*
 * LEDBlinker.cpp
 *
 *  Created on: 26.04.2017
 *      Author: Bartosz Bielawski
 */

#include "LEDBlinker.h"
#include "Arduino.h"
#include "config.h"
#include "utils.h"

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
