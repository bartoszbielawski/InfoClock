/*
 * LEDBlinker.h
 *
 *  Created on: 26.04.2017
 *      Author: Bartosz Bielawski
 */

#ifndef LEDBLINKER_H_
#define LEDBLINKER_H_

#include "C-Tasks/task.hpp"

class LEDBlinker: public Tasks::Task
{
	public:
		LEDBlinker();
		virtual void run();
		virtual ~LEDBlinker() = default;
	private:
		bool s = false;
};

#endif /* LEDBLINKER_H_ */
