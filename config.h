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

//SPI DISPLAY
const static uint8_t LED_CS = D8;
//Not used anymore as hardware SPI is used now
//LED_MOSI = D7
//LED_CLK = D5

const static int32_t DISPLAYS = 5;
const static uint8_t INTENSITY = 0;

const static int32_t MS_PER_CYCLE = 10;

//Local Sensor Task
const static uint8_t ONE_WIRE_TEMP = D3;
//use this define if you have no free ground pin and want to use some DIO
#define OW_GND D2

const static char versionString[] = "v 0.2.2";

#endif /* CONFIG_H_ */
