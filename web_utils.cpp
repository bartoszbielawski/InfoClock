/*
 * web_utils.cpp
 *
 *  Created on: 04.05.2017
 *      Author: caladan
 */



#include "web_utils.h"
#include "utils.h"
#include "MacroStringReplace.h"
#include "html/webpage.h"
#include "ESP8266WebServer.h"

FlashStream pageHeaderFS(pageHeader);

const char textPlain[] = "text/plain";
const char textHtml[] = "text/html";

bool handleAuth(ESP8266WebServer& webServer)
{
	bool authed = webServer.authenticate("user", readConfig(F("configPassword")).c_str());
	if (!authed)
		webServer.requestAuthentication();

	return authed;
}
