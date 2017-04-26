/*
 * task.hpp
 *
 *  Created on: 12.2016
 *      Author: Bartosz Bielawski
 */

#pragma once

#ifndef TASK_HPP
#define TASK_HPP

#include <cstdint>
#include <functional>
#include <type_traits>

#define TASKS_ERASE

#ifdef TASKS_ERASE
#include <algorithm>
#endif

namespace Tasks
{
	enum class State
	{
			READY,
			SLEEPING,
			SUSPENDED,
			DEAD,
			WAITING
	};

	template <class CondType>
	class Condition
	{
		public:
			Condition(): _cond() {}
			void set(CondType cond) {_cond = cond;}
			bool check() {return _cond();}
		private:
			CondType _cond;
	};

	template <>
	class Condition<void>
	{
		public:
			template <class T>
			void set(T t) {}
			bool check() {return false;}
	};

	class Task
	{
		public:
			Task(): _state(State::READY), _timer(0) {}
			virtual ~Task() = default;

			Task& operator=(Task& other) = delete;
			Task(Task& other) = delete;

			virtual void run() = 0;
			virtual bool checkCondition() {return false;}
			virtual void reset() {}

			State getState() const {return _state;}

			void resume()  {_state = State::READY;}
			void suspend() {_state = State::SUSPENDED;}
			void sleep(uint32_t ticks) {_timer = ticks; _state = State::SLEEPING;}
			void kill() {_timer = 0; _state = State::DEAD;}

			bool timedOut() const {return _timer == 0;}
			uint32_t decreaseTimer() {return --_timer;}

		protected:
			void _wait(uint32_t ticks) {_timer = ticks; _state = State::WAITING;}

			State    _state;
			uint32_t _timer;
	};

	template <class T>
	using TaskState = void (T::*)();

	using SimpleCond = bool (*)();
	using LambdaCond = std::function<bool()>;

	template <class T, class CondType = void>
	class TaskCRTP: public Task
	{
		public:
			TaskCRTP(TaskState<T> s):  nextState(s) {}
			virtual ~TaskCRTP() {}

			virtual void run()
			{
				(static_cast<T*>(this)->*nextState)();
			}

			virtual bool checkCondition()
			{
				return condition.check();
			}

			template <class Q = CondType>
			typename std::enable_if<!std::is_same<Q, void>::value, void>
			wait(uint32_t ticks, Q cond)
			{
					condition.set(cond);
					_wait(ticks);
			}

		protected:
			TaskState<T> nextState;
			Condition<CondType> condition;
	};


	void schedule(Task* t);

	template <class T>
	void schedule(const T& container)
	{
			for (auto& e: container)
			{
				schedule(e);
			}
	}

template <class T>
void removeDead(T& container)
{
		container.erase(std::remove_if(std::begin(container), std::end(container), [](typename T::value_type& v) {
			auto s = v->getState();
			return s == State::DEAD;
		}), container.end());
}


void updateSleep(Task* t);

template <class T>
void updateSleep(const T& container)
{
	for (auto& e: container)
	{
		updateSleep(e);
	}
}

}	//end namespace


#endif //TASK_HPP
