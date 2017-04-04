/*
 * LocalSensorTask.h
 *
 *  Created on: 04.04.2017
 *      Author: caladan
 */

#ifndef LOCALSENSORTASK_H_
#define LOCALSENSORTASK_H_

#include "task.hpp"
#include "OneWire.h"
#include "DallasTemperature.h"



class LocalSensorTask: public Tasks::Task
{
	public:
		LocalSensorTask();
		virtual ~LocalSensorTask();

		virtual void run();
	private:
		OneWire oneWire;
		DallasTemperature dallasTemperature;
};

#endif /* LOCALSENSORTASK_H_ */
