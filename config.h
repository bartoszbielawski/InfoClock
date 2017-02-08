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

const static uint8_t LED_DTA = D3;
const static uint8_t LED_CS = D2;
const static uint8_t LED_CLK = D1;

const static int32_t MS_PER_CYCLE = 20;

const static int32_t DISPLAYS=8;

const static char versionString[] = "0.1.0 (" __DATE__ " " __TIME__ ")";

#endif /* CONFIG_H_ */
