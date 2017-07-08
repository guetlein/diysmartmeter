//############################################################
//			config
//defines all Parameters that can be set via webinterface
//############################################################

#ifndef config_h
#define config_h

enum ProcessMode {
	FixedOutputInterval,   //Publish every n seconds
	DynamicOutputInterval  //Publish if delta > n
};

enum DebugLevel {
	debugOff, debugError, debugWarning, debugInfo
};

const int MAXPATTERN = 10;

//with default values
typedef struct {

	String deviceID = "-?-";
	DebugLevel debugLevel = debugInfo;
	int baud = 9600;

	//gather
	String patternBegin[MAXPATTERN] =
			{ "0100010801", "0100010802", "0100020801", "0100020802",
					"0100100700", "0100010800", "0100020800" };
	String patternEnd[MAXPATTERN] = { "01", "01", "01", "01", "01", "01", "01" };
	String patternLabel[MAXPATTERN] = { "Bezug 1", "Bezug 2", "Lieferung 1",
			"Lieferung 2", "Wirkleistung", "Bezug Total", "Lieferung Total" };
	int patternLength[MAXPATTERN] = { -1, -1, -1, -1, -1, -1, -1 };
	int patternTypeID[MAXPATTERN] = { 29, 31, 30, 32, 33, 27, 28 };
	int patternOffset[MAXPATTERN] = { 22, 22, 22, 22, 22, 22, 22 };
	int patternCount = 7;
	String webUITelegram = "";

	//process
	uint intervalParameter = 5;
	ProcessMode intervalMode = FixedOutputInterval;

	//publish
	bool mqtt_active = true;
	String mqtt_server = "localhost";
	String mqtt_topic = "mytopic";
	int mqtt_port = 1883;
	bool mqtt_includeDeviceID = true;
	bool mqtt_includeLabel = true;
	bool mqtt_useSubTopics = false;
	bool rest_active = true;
	String rest_server = "localhost";

} Config;

extern Config config;

#endif
