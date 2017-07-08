/**************************************************************
 WebConfig is based on WifiManager
 WiFiManager is a library for the ESP8266/Arduino platform
 (https://github.com/esp8266/Arduino) to enable easy
 configuration and reconfiguration of WiFi credentials using a Captive Portal
 inspired by:
 http://www.esp8266.com/viewtopic.php?f=29&t=2520
 https://github.com/chriscook8/esp-arduino-apboot
 https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
 Built by AlexT https://github.com/tzapu
 Licensed under MIT license

 Modified by moritz
 **************************************************************/

#ifndef WebConfig_h
#define WebConfig_h
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

//#include <../../diysmartmeter/base.h>
#include <memory>

extern "C" {
#include "user_interface.h"
}

const char HTTP_HEAD[] PROGMEM = " \n   \n<html>\n<head>\n<title>{v}</title> \n";
const char HTTP_JS1[] PROGMEM = "<script> \n function getTelegram(){ \n 	var t = document.getElementById('telegram').value; \n	 	return t; \n } \n function h2a(h) { \n var hex = h.toString(); \n var str = ''; \n for (var i = 0; i < hex.length; i += 2) \n 	str += String.fromCharCode(parseInt(hex.substr(i, 2), 16)); \n return str; \n } \n function getVendor(){\n var telegram = getTelegram();\n var len = -1;\n var begin = '77078181c78203ff0101010104';\n	 var end = '01';\n var skip = 0;\n if(telegram.search(begin) == -1){\n 		return '--'; 	\n } 	\n var pos_b = telegram.search(begin) + begin.length + skip; 	\n if(telegram.substring(pos_b).search(end) == -1){ 		\n 	return '--'; \n }\n var pos_e = telegram.substring(pos_b).search(end); \n if(len > 0 && len < 100 && pos_e > len){\n 		pos_e = len;\n }\n pos_e = pos_e + pos_b; \n if(pos_b>0 && pos_e >0) { \n 	return h2a(telegram.substring(pos_b, pos_e)); \n 	} \n return '--';\n } function getSerial(){\n var telegram = getTelegram();\n var len = '-1';\n var begin = '77070100000009ff010101010b';\n var end = '01';\n var skip = 0;\n if(telegram.search(begin) == -1){ \n 		return '--'; \n }\n var pos_b = telegram.search(begin) + begin.length + skip; \n if(telegram.substring(pos_b).search(end) == -1){ \n 	return '--'; \n } var pos_e = telegram.substring(pos_b).search(end); \n if(len > 0 && len < 100 && pos_e > len){ 	\n 	pos_e = len;\n }\n pos_e = pos_e + pos_b;\n if(pos_b>0 && pos_e >0) {\n 		return telegram.substring(pos_b, pos_e);\n }	\n return '--'; \n }\n function r(){\n 	var vendor = getVendor();\n 	var serial = getSerial();\n 	document.getElementById('vendor').value = vendor;\n 	document.getElementById('serial').value = serial;\n 	document.getElementById('device').value = vendor + '_' + serial;\n 	document.getElementById('baudrate').value = document.getElementById('baud').value;\n 	var telegram = getTelegram();\n 	for(var i = 0; i<";
const char HTTP_JS2[] PROGMEM = " ;i++) {\n var c = 'u' + i; var o = 'v' + i; 		var r = 'r' + i; 		var len = 'le' + i; \n 	 len = document.getElementById(len).value;\n 		document.getElementById(o).value = '--';\n 		var begin = 'b' + i;\n 		var end = 'e' + i;\n 		var skip = 's' + i;\n 		begin = document.getElementById(begin).value;\n 	if(begin == null || begin == ''){\n 		continue;\n 		} 		skip = parseInt(document.getElementById(skip).value);\n 		end = parseInt(document.getElementById(end).value);\n 	if(telegram.search(begin) == -1){\n 		continue;\n 		}\n 		var pos_b = telegram.search(begin) + begin.length + skip;\n 	if(telegram.substring(pos_b).search(end) == -1){\n 		continue;\n 		}else{ }\n 		var pos_e = telegram.substring(pos_b).search(end)-1;\n 	if(len > 0 && len < 100 && pos_e > len){\n 		pos_e = len;\n 		}\n 		pos_e = pos_e + pos_b;\n 	if(pos_b>0 && pos_e >0) {\n 		document.getElementById(o).value = parseInt(telegram.substring(pos_b, pos_e),16);\n 		document.getElementById(r).value = telegram.substring(pos_b, pos_e);\n 		if(document.getElementById(o).value != ''){\n 			document.getElementById(c).checked=true;\n 		}\n 		}\n 	}\n }\n window.onload = function() {\n r();\n }\n function setBaud(){\n 	document.getElementById('baudrate').value = document.getElementById('baud').value;\n }\n </script> ";
const char HTTP_GATH[] PROGMEM = "</textarea>\n</form>\n</div>\n<div style='width:800px;margin:0 auto; display:block;text-align:left;'>\n<form action=\"/gatherSave\" method=\"get\">\n<input type='hidden' name='baudrate' id='baudrate' value='9600'/><table style='margin-bottom:30px'><tr><td>Vendor ID:</td><td><input type='text' name='vendor' id='vendor' style='width:250px;display:inline;margin:5px;height:25px' /></td></tr>\n<tr><td>Serial ID:</td><td><input type='text' name='serial' id='serial' style='width:250px;display:inline;margin:5px;height:25px' /></td></tr>\n<tr><td>Proposed Device-ID:</td><td><input type='text' name='device' id='device' style='width:250px;display:inline;margin:5px;height:25px' /></td></tr></table>\n\n<table style='font-size:10px'>\n<tr><td>Use</td><td>TID</td><td>Label</td><td>Value</td><td>Raw</td><td>Startpattern</td><td>Stoppattern</td><td>Offset</td><td>Length</td></tr>\n";
const char HTTP_BAUD1[] PROGMEM = "<div style='width:800px;margin:0 auto; display:block;text-align:left;'>\n<form action=\"/rt\" method=\"get\">\n<div style='width:100%;text-align:left;'>Baudrate: "; //<select style='width:100px;margin:5px;height:32px;' name='baud' id='baud' onchange='setBaud()'>";
const char HTTP_BAUD2[] PROGMEM = "<input type='submit' style='width:250px;display:inline;margin-left:15px;height:32px' value='Try to read in bits for 10 seconds!' /></div>\n<textarea id=\"telegram\" rows='8' style='width:800px;display:block;'>";
const char HTTP_STYLE[] PROGMEM = "\n<style>\nbody,table{font-size: 12px; font-family: verdana;} h1{color:#666666; font-family:arial,sans-serif; font-size:32px; border-bottom:1px solid;} \n button { width:200px; display:block;box-shadow:inset 0px 1px 0px 0px #ffffff; background:linear-gradient(to bottom, #f9f9f9 5%, #e9e9e9 100%); background-color:#f9f9f9; 	-moz-border-radius:6px; -webkit-border-radius:6px; 	border-radius:6px; 	border:1px solid #dcdcdc; 	display:inline-block; 	cursor:pointer; \n	color:#666666; 	font-weight:bold; 	padding:6px 24px; 	text-decoration:none; 	text-shadow:0px 1px 0px #ffffff; } \n button:hover { 	background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #e9e9e9), color-stop(1, #f9f9f9)); 	background:-moz-linear-gradient(top, #e9e9e9 5%, #f9f9f9 100%); background:linear-gradient(to bottom, #e9e9e9 5%, #f9f9f9 100%); background-color:#e9e9e9; }\n</style>";
const char HTTP_SCRIPT[] PROGMEM = "\n<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM = "\n</head>\n<body>\n\<div style='text-align:left;width:830px; margin:0 auto;'><h1>DIY SmartMeter</h1>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM = "<div style='width:200px;margin:0 auto; display:block;text-align:left;'><form action=\"/gather\" method=\"get\"><button>Configure</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/restart\" method=\"post\"><button>Restart</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset Config</button></form></div>";
const char HTTP_ITEM[] PROGMEM = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM = "<form method='get' action='/wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='Password'><p><input type='submit' value='Continue' /></p></form>";
const char HTTP_FORM_PARAM[] PROGMEM = "<br/><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM = " ";
const char HTTP_SCAN_LINK[] PROGMEM = "<div class=\"c\"><a href=\"/wifi\">Scan</a></div><br/><br/>";
const char HTTP_SAVED[] PROGMEM = "<div>Configuration done!<br />No need for restart, proccessing started right now.</div>";
const char HTTP_END[] PROGMEM = "</div></body></html>";

#define WIFI_MANAGER_MAX_PARAMS 10

class WebConfigParameter {
public:
	WebConfigParameter(const char *custom);
	WebConfigParameter(const char *id, const char *placeholder,
			const char *defaultValue, int length);
	WebConfigParameter(const char *id, const char *placeholder,
			const char *defaultValue, int length, const char *custom);

	const char *getID();
	const char *getValue();
	const char *getPlaceholder();
	int getValueLength();
	const char *getCustomHTML();
private:
	const char *_id;
	const char *_placeholder;
	char *_value;
	int _length;
	const char *_customHTML;
	void init(const char *id, const char *placeholder, const char *defaultValue,
			int length, const char *custom);

	friend class WebConfig;
};

class WebConfig {
public:
	WebConfig();
	boolean autoConnect();
	boolean autoConnect(char const *apName, char const *apPassword = NULL);

	//if you want to always start the config portal, without trying to connect first
	boolean startConfigPortal(char const *apName,
			char const *apPassword = NULL);

	// get the AP name of the config portal, so it can be used in the callback
	String getConfigPortalSSID();

	void resetSettings();

	//sets timeout before webserver loop ends and exits even if there has been no setup.
	//usefully for devices that failed to connect at some point and got stuck in a webserver loop
	//in seconds setConfigPortalTimeout is a new name for setTimeout
	void setConfigPortalTimeout(unsigned long seconds);
	void setTimeout(unsigned long seconds);

	//sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
	void setConnectTimeout(unsigned long seconds);

	void setDebugOutput(boolean debug);
	//defaults to not showing anything under 8% signal quality if called
	void setMinimumSignalQuality(int quality = 8);
	//sets a custom ip /gateway /subnet configuration
	void setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
	//sets config for a static IP
	void setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
	//called when AP mode and config portal is started
	void setAPCallback(void (*func)(WebConfig*));
	//called when settings have been changed and connection was successful
	void setSaveConfigCallback(void (*func)(void));
	//called when config needs reset
	void setResetConfigCallback(void (*func)(void));
	//called when webcounter needs reset
	void setResetWebCounterCallback(void (*func)(void));
	//called for logging purposes
	void setLogCallback(void (*func)(String));
	//called when to read new telegram
	void setReadTelegramCallback(void (*func)(int));
	//called to write config
	void setWriteConfigCallback(void (*func)(void));
	//adds a custom parameter
	void addParameter(WebConfigParameter *p);
	//if this is set, it will exit after config, even if connection is unsucessful.
	void setBreakAfterConfig(boolean shouldBreak);
	//if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
	//TODO
	//if this is set, customise style
	void setCustomHeadElement(const char* element);
	//if this is true, remove duplicated Access Points - defaut true
	void setRemoveDuplicateAPs(boolean removeDuplicates);

private:
	std::unique_ptr<DNSServer> dnsServer;
	std::unique_ptr<ESP8266WebServer> server;

	//const int WM_DONE = 0;
	//const int WM_WAIT = 10;

	//const String HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

	void setupConfigPortal();
	void startWPS();

	const char* _apName = "no-net";
	const char* _apPassword = NULL;
	String _ssid = "";
	String _pass = "";
	unsigned long _configPortalTimeout = 0;
	unsigned long _connectTimeout = 0;
	unsigned long _configPortalStart = 0;

	IPAddress _ap_static_ip;
	IPAddress _ap_static_gw;
	IPAddress _ap_static_sn;
	IPAddress _sta_static_ip;
	IPAddress _sta_static_gw;
	IPAddress _sta_static_sn;

	int _paramsCount = 0;
	int _minimumQuality = -1;
	boolean _removeDuplicateAPs = true;
	boolean _shouldBreakAfterConfig = false;
	boolean _tryWPS = false;

	const char* _customHeadElement = "";

	//String getEEPROMString(int start, int len);
	//void setEEPROMString(int start, int len, String string);

	int status = WL_IDLE_STATUS;
	int connectWifi(String ssid, String pass);
	uint8_t waitForConnectResult();

	void handleRoot();
	void handleWifi(boolean scan);
	void handleWifiSave();
	void handleInfo();
	void handleReset();
	void handleRestart();
	void handleNotFound();
	void handle204();
	void handleReadTelegram();
	void handlePublishSave();
	void handleGatherSave();
	void handleProcessSave();
	void handlePublish();
	void handleGather();
	void handleProcess();
	boolean captivePortal();

	// DNS server
	const byte DNS_PORT = 53;

	//helpers
	int getRSSIasQuality(int RSSI);
	boolean isIp(String str);
	String toStringIp(IPAddress ip);

	boolean connect;
	boolean _debug = true;

	void (*_apcallback)(WebConfig*) = NULL;
	void (*_savecallback)(void) = NULL;
	void (*_readTelegramCallback)(int) = NULL;
	void (*_writeConfigCallback)(void) = NULL;
	void (*_resetConfigCallback)(void) = NULL;
	void (*_resetWebCounterCallback)(void) = NULL;
	void (*_logCallback)(String) = NULL;

	WebConfigParameter* _params[WIFI_MANAGER_MAX_PARAMS];

	template<typename Generic>
	void DEBUG_WM(Generic text);

	template<class T>
	auto optionalIPFromString(T *obj,
			const char *s) -> decltype( obj->fromString(s) ) {
		return obj->fromString(s);
	}
	auto optionalIPFromString(...) -> bool {
		DEBUG_WM(
				"NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
		return false;
	}

};

#endif
