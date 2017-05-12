/*
 * SyslogSender.hxx
 *
 *  Created on: 01.05.2017
 *      Author: caladan
 */

#ifndef SYSLOGSENDER_HXX_
#define SYSLOGSENDER_HXX_

#include "Arduino.h"

extern String syslogServer;

void syslogSend(const String& app, const char* msg);

#endif /* SYSLOGSENDER_HXX_ */
