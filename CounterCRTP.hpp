/*
 * CounterCRTP.hpp
 *
 *  Created on: 22.05.2017
 *      Author: caladan
 */

#ifndef COUNTERCRTP_HPP_
#define COUNTERCRTP_HPP_


template <class T>
class CounterCRTP
{
	public:
		CounterCRTP(): counter(getNext()) {}
		uint32_t getId() const {return counter;}

	protected:
		uint32_t counter;

	private:
		static uint32_t getNext()
		{
			static uint32_t cntr = 0;
			cntr++;
			return cntr;
		}
};


#endif /* COUNTERCRTP_HPP_ */
