/*
 * LHCStatusReaderNew.h
 *
 *  Created on: 19.10.2017
 *      Author: Bartosz Bielawski
 */

#ifndef LHCSTATUSREADERNEW_H_
#define LHCSTATUSREADERNEW_H_

#include <tasks.hpp>

#include <ESP8266WebServer.h>
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
