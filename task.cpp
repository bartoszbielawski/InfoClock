/*
 * task.cpp
 *
 *  Created on: 26.04.2017
 *      Author: caladan
 */

#include "task.hpp"

void Tasks::updateSleep(Task* t)
{
	if (!t)
		return;

	switch (t->getState())
	{
		case State::WAITING:
			if (t->checkCondition())
			{
				t->resume();
				break;
			}
			//no break
		case State::SLEEPING:
			if (!t->decreaseTimer())
				t->resume();
			break;

		default:
			break;
	}
}

void Tasks::schedule(Task* t)
{
	if (t == nullptr)
		return;

	if (t->getState() == State::READY)
		t->run();
}
