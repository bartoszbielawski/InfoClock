/*
 * wifiConnector.h
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#ifndef WIFICONNECTOR_H_
#define WIFICONNECTOR_H_

#include <vector>

#include "ESP8266WiFiMulti.h"
#include "ESP8266WiFi.h"

#include "task.hpp"
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

		WifiConnector(Callback callback): TaskCRTP(&WifiConnector::lateInit), callback(callback)
		{
		}

		void initAP()
		{
			//run the AP
			WiFi.mode(WIFI_AP);
			WiFi.softAP("esp-display");

			//notify tasks we're AP now
			callback(States::AP);
			logPrintf("Running in AP mode...");

			//wake up in 60 seconds and try to reconnect once again as a client
			nextState = &WifiConnector::lateInit;
			mainState = States::CLIENT;
			sleep(300_s);
			return;
		}

		void initSTA(const String& essid)
		{

			WiFi.softAPdisconnect();
			WiFi.disconnect();
			WiFi.mode(WIFI_STA);

			String pwd   = readConfig("wifiPassword");

			logPrintf("Running in CLIENT mode...");
			logPrintf("Connecting to %s - %s", essid.c_str(), pwd.c_str());

			WiFi.begin(essid.c_str(), pwd.c_str());
			nextState = &WifiConnector::monitorClientStatus;
		}

		void lateInit()
		{
			connectionTimeout = 60;
			if (callback)
				callback(States::NONE);

			auto essid = readConfig("essid");

			if (essid.length() == 0)
				mainState = States::AP;

			switch (mainState)
			{
				case States::AP:
					initAP();
					break;
				case States::CLIENT:
					initSTA(essid);
					break;
			}
		}

		void monitorClientStatus()
		{
			auto status = WiFi.status();

			//check the status, notify about state change
			if (connected != (status == WL_CONNECTED))
			{
				connected = (status == WL_CONNECTED);
				if (callback)
				{
					callback(connected ? States::CLIENT: States::NONE);
				}
			}

			//handle timeout...
			connectionTimeout = connected ? 60: std::max(connectionTimeout-1, 0);

			if (connectionTimeout == 0)
			{
				logPrintf("Timed out - falling back into AP mode...");
				mainState = States::AP;
				nextState = &WifiConnector::lateInit;
				return;
			}
			sleep(connected ? 10_s: 1_s);
		}

		bool getConnected() const {return connected;}

	private:
		States				mainState = States::CLIENT;
		Callback 			callback = nullptr;
		bool 				connected = false;
		wl_status_t 		status;
		int32_t				connectionTimeout = 60;
};




#endif /* WIFICONNECTOR_H_ */
