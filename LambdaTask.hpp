/*
 * LambdaTask.hpp
 *
 *  Created on: 24.05.2017
 *      Author: caladan
 */

#ifndef LAMBDATASK_HPP_
#define LAMBDATASK_HPP_

#include "task.hpp"
#include <functional>

class LambdaTask: public Tasks::Task
{
	public:
		LambdaTask(std::function<void()> f): f(f) {}

		virtual void run()
		{
			f();
			suspend();
			//TODO: the task should be deleted!
		}

		virtual ~LambdaTask() = default;

	private:
		std::function<void()> f;

};



#endif /* LAMBDATASK_HPP_ */
