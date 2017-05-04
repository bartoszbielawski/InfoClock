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
  .label {text-align: right; font-style: italic;}
  .wide {font-weight: bold; font-size: 130%;}
  a {text-decoration: none;}
</style></head>
<body>
<a href="/">&lArr;	</a>
)_";

static const char generalSettingsPage[] PROGMEM = R"_(
<form method="post" action="settings" autocomplete="on">
<table>
<tr><td class="wide">WiFi settings</td></tr>
<tr><td class="label">ESSID:</td><td><input name="essid" type="text" value="$essid$"></td></tr>
<tr><td class="label">Password:</td><td><input name="wifiPassword" type="password"></td></tr>
<tr><td class="wide">Timezone settings</td></tr>
<tr><td class="label">In seconds including DST:</td><td><input name="timezone" type="text" value="$timezone$"></td></tr>
<tr><td class="wide">Logging settings</td></tr>
<tr><td class="label">Syslog server:</td><td><input name="syslogServer" type="text" value="$syslogServer$"></td></tr>
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
      <tr><td class="wide">Webmessage</td></tr>
      <tr><td class="label">Message:</td><td><input name="webmessage" type="text" value="$webmessage$"></td></tr>
      <tr><td/><td><input type="submit"></td></tr>
    </table>
    </form>
  </body>
</html>
)_";



static const char statusPage[] PROGMEM = R"_(
<form method="post" action="owm" autocomplete="on">
<table>
<tr><td class="wide">General</td><tr>
<tr><td class="label">Version:</td><td>$version$</td></tr>
<tr><td class="label">Free heap:</td><td>$heap$</td></tr>
<tr><td class="label">Up time:</td><td>$uptime$</td></tr>
<tr><td class="wide">WiFi</td></tr>
<tr><td class="label">essid:</td><td>$essid$</td></tr>
<tr><td class="label">IP:</td><td>$ip$</td></tr>
<tr><td class="label">MAC Address:</td><td>$mac$</td></tr>
<tr><td class="wide">Weather</td></tr>
<tr><td class="label">Location:</td><td>$OWM.Location$</td></tr>
<tr><td class="label">Temperature:</td><td>$OWM.Temperature$</td></tr>
<tr><td class="label">Pressure:</td><td>$OWM.Pressure$</td></tr>
</table>
</form>
</body>
</html>
)_";

static const char mainPage[] PROGMEM = R"_(
<html>
  <head>
    <meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP Display</title>
    <style>
      a {text-align: center; display: block; font-size: 150%; color: #353; text-decoration: none; background-color: #DFD;
             padding: 4px; border: 1px solid #383; border-radius: 10px; max-width: 400px}
      a:hover {background-color: #4d4;}
      h2 {color: green; text-align: center; max-width: 400px}
      </style>
  </head>
  <body>
   <h2>ESP Display</h2>
   <a href="/settings">General settings</a>
   <a href="/owm">OWM settings</a>
   <a href="/webmessage">Web Message</a>
   <a href="/status">Device status</a>
  </body>
</html>
)_";


#endif /* HTML_WEBPAGE_H_ */
