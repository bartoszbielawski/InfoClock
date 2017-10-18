/*
 * LHCStatusReaderNew.h
 *
 *  Created on: 11.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef LHCSTATUSREADERNEW_H_
#define LHCSTATUSREADERNEW_H_

#include <task.hpp>

class ESP8266WebServer;

class LHCStatusReaderNew: public Tasks::Task
{
	public:
		LHCStatusReaderNew();
		virtual void reset();
		virtual void run();
		virtual ~LHCStatusReaderNew() = default;
	private:
		String beamEnergy;
		String beamMode;
		String page1Comment;
		String refreshTime;

		void handleStatusPage(ESP8266WebServer& ws);

		String getEnergy();
		String getStateInfo();
};

#endif /* LHCSTATUSREADERNEW_H_ */
