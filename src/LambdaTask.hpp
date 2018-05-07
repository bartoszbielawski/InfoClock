/*
 * LambdaTask.hpp
 *
 *  Created on: 24.05.2017
 *      Author: caladan
 */

#ifndef LAMBDATASK_HPP_
#define LAMBDATASK_HPP_

#include <tasks.hpp>
#include <functional>

class LambdaTask: public Tasks::Task
{
	public:
		LambdaTask(std::function<void()> f): f(f) {}

		virtual void run()
		{
			f();
			kill();
			//TODO: think about task clean-up
		}

		virtual ~LambdaTask() = default;

	private:
		std::function<void()> f;

};



#endif /* LAMBDATASK_HPP_ */
