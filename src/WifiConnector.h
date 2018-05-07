/*
 * wifiConnector.h
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#ifndef WIFICONNECTOR_H_
#define WIFICONNECTOR_H_

#include <vector>

#include "ESP8266WiFi.h"
#include "Arduino.h"
#include <tasks.hpp>
#include "utils.h"

class WifiConnector: public Tasks::TaskCRTP<WifiConnector>
{
	public:
		enum class States
		{
			CLIENT,
			AP,
			NONE
		};

		using Callback = void (*)(States state);

		WifiConnector(Callback callback);
		void lateInit();
		void initAP();
		void initSTA(const String& essid);
		void monitorClientStatus();
		bool getConnected() const;

	private:
		States				mainState = States::CLIENT;
		Callback 			callback = nullptr;
		bool 				connected = false;
		wl_status_t 		status{};
		int32_t				connectionTimeout = 60;
};


#endif /* WIFICONNECTOR_H_ */
