/*
 * web_utils.h
 *
 *  Created on: 04.05.2017
 *      Author: caladan
 */

#ifndef WEB_UTILS_H_
#define WEB_UTILS_H_

#include "MacroStringReplace.h"

#include <ESP8266WebServer.h>

bool handleAuth(ESP8266WebServer& webServer);
extern FlashStream pageHeaderFS;

extern const char textPlain[];
extern const char textHtml[];


#endif /* WEB_UTILS_H_ */
