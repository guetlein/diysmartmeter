//############################################################
// 				base
//	~ all Parameters that cannot be changed by user
//	~ global vars and constants
//  ~ helper functions
//############################################################

#ifndef base_h
#define base_h

#include <SoftwareSerial.h>
#include <Chronos.h>
#include "config.h"

//debugging
const int swSerBaud = 115200;
//rx: d5 -> 14, tx: d6 -> 12
SoftwareSerial swSer(14, 12, false, 256);

//misc
const char* TIMEHOST = "europe.pool.ntp.org";
const int TIMEOFFSET = 7200; //in seconds
const int TIMEREFRESH = 10000; //in ms

//gather
String inbuffer = "";
const int TELETIMELIMIT = 10000;

//process
const char ENDCHAR = '\x01';
const String POSTFIX = "1b1b1b1b1a01";
const String PREFIX = "7707";
const int PROCESSBUFFERSIZE = 1024;
const int PROCLINELIMIT = 100;
const int MAXMEASURES = 32;
const int SPACE = 4098;

//publish
const int PUBLISHMAXTRIES = 5;

//connection
const String APPASS = "X_PASSWORD_TO_BE_SET_X";
const String APSSID = "X_SSID_TO_BE_SET_X";
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

//helperfunctions
void printCfg(Config* cfg, SoftwareSerial& s) {
	s.println("========================================");
	s.println("baud:                " + String(cfg->baud));
	s.println("debugLevel:          " + String(cfg->debugLevel));
	s.println("deviceID:            " + cfg->deviceID);
	s.println("intervalMode:        " + String(cfg->intervalMode));
	s.println("intervalParameter:   " + String(cfg->intervalParameter));
	s.println("mqtt_active:         " + String(cfg->mqtt_active));
	s.println("mqtt_includeDeviceID:" + String(cfg->mqtt_includeDeviceID));
	s.println("mqtt_includeLabel:   " + String(cfg->mqtt_includeLabel));
	s.println("mqtt_port:           " + String(cfg->mqtt_port));
	s.println("mqtt_server:         " + cfg->mqtt_server);
	s.println("mqtt_topic:          " + cfg->mqtt_topic);
	s.println("mqtt_useSubTopics:   " + String(cfg->mqtt_useSubTopics));
	s.println("rest_active:         " + String(cfg->rest_active));
	s.println("rest_server:         " + cfg->rest_server);
	s.println("patternCount:        " + String(cfg->patternCount));
	s.println("========================================");
}

String getFormatted(String t) {
	t.trim();
	int e = 0;
	String result;
	int pos = 0;
	String s = "7707";
	//split after logical lines
	do {
		int tmp = t.indexOf(s, pos);
		if (tmp > pos)
			e = tmp;
		else
			e = pos + 256;
		int b = pos;
		if (b > 0)
			b--;
		result += t.substring(b, e) + "\n";
		pos = e + 1;
	} while (pos < t.length());
	return result;
}

String getDateTime(uint m){
  Chronos::DateTime t(m);
  char buf[21];
  sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d", t.year(), t.month(), t.day(), t.hour(), t.minute(), t.second());
  return String(buf);
}

#endif
