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

//display settings
const static int32_t DISPLAYS = 8;
const static uint8_t INTENSITY = 2;

const static char versionString[] = "v 0.1.4";

#endif /* CONFIG_H_ */
