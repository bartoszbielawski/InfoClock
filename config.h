/*
 * config.h
 *
 *  Created on: 10.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "Arduino.h"

const static uint8_t LBLUE = D4;

const static uint8_t LED_CS = D8;

const static int32_t MS_PER_CYCLE = 20;

const static int32_t DISPLAYS=5;

const static char versionString[] = "0.1.2 (" __DATE__ " " __TIME__ ")";

#endif /* CONFIG_H_ */
