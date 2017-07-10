/*
 * webpage.h
 *
 *  Created on: 01.02.2017
 *      Author: Bartosz Bielawski
 */

#ifndef HTML_WEBPAGE_H_
#define HTML_WEBPAGE_H_

#include <pgmspace.h>

static const char pageHeader[] PROGMEM = R"_(
<html><head>
<meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
<title>$title$</title><style>
  h2 {color: green;}
  th {font-weight: bold; font-size: 130%;}
  .l {text-align: right; font-style: italic;}
  a {text-decoration: none;}
</style></head>
<body>
<a href="/">&lArr;	</a>
)_";

static const char generalSettingsPage[] PROGMEM = R"_(
<form method="post" action="settings" autocomplete="on">
<table>
<tr><th>Password</th></tr>
<tr><td class="l">Password:</td><td><input name="configPassword" type="password"></td></tr>
<tr><th>WiFi settings</th></tr>
<tr><td class="l">ESSID:</td><td><input name="essid" type="text" value="$essid$"></td></tr>
<tr><td class="l">Password:</td><td><input name="wifiPassword" type="password"></td></tr>
<tr><th>Timezone settings</th></tr>
<tr><td class="l">In seconds including DST:</td><td><input name="timezone" type="text" value="$timezone$"></td></tr>
<tr><th>Logging settings</th></tr>
<tr><td class="l">Syslog server:</td><td><input name="syslogServer" type="text" value="$syslogServer$"></td></tr>
<tr><td/><td><input type="submit"></td></tr>
</table>
<input type="hidden" name="submitted" value="true">
</form>
</body>
</html>
)_";


static const char webmessagePage[] PROGMEM = R"_(
    <form method="post" action="webmessage" autocomplete="on">
    <table>
      <tr><th>Webmessage</th></tr>
      <tr><td class="l">Message:</td><td><input name="webmessage" type="text" value="$webmessage$"></td></tr>
      <tr><td/><td><input type="submit"></td></tr>
    </table>
    </form>
  </body>
</html>
)_";



static const char statusPage[] PROGMEM = R"_(
<table>
<tr><th>General</th><tr>
<tr><td class="l">Version:</td><td>$version$</td></tr>
<tr><td class="l">Free heap:</td><td>$heap$</td></tr>
<tr><td class="l">Up time:</td><td>$uptime$</td></tr>
<tr><th>WiFi</th></tr>
<tr><td class="l">essid:</td><td>$essid$</td></tr>
<tr><td class="l">IP:</td><td>$ip$</td></tr>
<tr><td class="l">MAC Address:</td><td>$mac$</td></tr>
</table>
</body>
</html>
)_";

static const char mainPage[] PROGMEM = R"_(
<html>
  <head>
    <meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
    <title>InfoClock</title>
    <style>
      a {text-align: center; display: block; font-size: 150%; color: #353; text-decoration: none; background-color: #DFD;
             padding: 4px; border: 1px solid #383; border-radius: 10px; max-width: 400px}
      a:hover {background-color: #4d4;}
      h2 {color: green; text-align: center; max-width: 400px}
      </style>
  </head>
  <body>
   <h2>InfoClock</h2>
   <a href="/status">Device status</a>
   <a href="/settings">General settings</a>
   <a href="/webmessage">Web Message</a>
   $links$
   <a href="/reset">Reset device</a>
  </body>
</html>
)_";


#endif /* HTML_WEBPAGE_H_ */
