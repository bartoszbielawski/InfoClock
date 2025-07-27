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

//SPI display configuration
//commented pins are hardware SPI
//LED_CLK = D5 - to clock (CLK)
//LED_MOSI = D7 - to data in (DIN)
const static uint8_t LED_CS = D8;		//to SS

//if you have two back-to-back connected displays put in series set it to 2
#define LED_DISPLAYS 1

const static int32_t MS_PER_CYCLE = 10;

//Local Sensor Task
const static uint8_t ONE_WIRE_TEMP = D3;
//use this define if you have no free ground pin and want to use some DIO
#define OW_GND D2

const static char versionString[] = "v 0.5.7";

const static char DEFAULT_USER[] = "user";

const static char HOSTNAME[] = "infoclock";

const static int SERIAL_BAUD_RATE = 115200;
#endif /* CONFIG_H_ */
