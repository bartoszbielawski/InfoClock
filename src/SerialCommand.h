/*
 * SerialCommand.h
 *
 *  Created on: 30.04.2020
 *      Author: Bartosz Bielawski
 */

#ifndef SERIALCOMMAND_H
#define SERIALCOMMAND_H

#include <tasks.hpp>
#include <Arduino.h>

class SerialCommandTask: public Tasks::Task
{
	public:
		SerialCommandTask();
		virtual void run();
		virtual ~SerialCommandTask() = default;
	private:
        String cumulatedInput;
};

#endif /* SERIALCOMMAND_H */
