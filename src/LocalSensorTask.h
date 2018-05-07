/*
 * LocalSensorTask.h
 *
 *  Created on: 04.04.2017
 *      Author: caladan
 */

#ifndef LOCALSENSORTASK_H_
#define LOCALSENSORTASK_H_

#include <tasks.hpp>
#include <OneWire.h>
#include <DallasTemperature.h>



class LocalSensorTask: public Tasks::Task
{
	public:
		LocalSensorTask();
		virtual ~LocalSensorTask() = default;

		virtual void run();

		float temperature = 0.0f;

	private:
		OneWire oneWire;
		DallasTemperature dallasTemperature;
};

#endif /* LOCALSENSORTASK_H_ */
