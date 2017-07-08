/**************************************************************
 diysmartmeter is a program running on the ESP8266/Arduino platform
 enabling smartmeter functions on meters with d0 interface
 
 Built by moritz
 **************************************************************/

#include <NTPClient.h>
#include <WiFiUdp.h>
#include "base.h"
#include "measure.h"
#include "config.h"
#include "gatherData.h"
#include "processData.h"
#include "publishData.h"
#include "configManager.h"
#include "webConfig.h"

bool webConfig = false;
Config config;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, TIMEHOST, TIMEOFFSET, TIMEREFRESH);

void setup() {

	//init comm
	Serial.begin(config.baud);
	swSer.begin(swSerBaud);
	delay(1000);

	//reset
	int f = getWebConfigFlag() + 1;
	setWebConfigFlag(f);
	if(f > 2){
		webConfig = true;
	}

	if(config.debugLevel >= debugInfo){
		swSer.println("[Main] Started up. Init... ");
	}

	//load config
	configManager(webConfig);

	//allocate space
	uint freeheap = ESP.getFreeHeap();
	int s = sizeof(Measure);
	freeheap -= SPACE;
	freeheap /= s;
	measure_maxSize = (freeheap < MAXMEASURES) ? freeheap : MAXMEASURES;
	if(config.debugLevel >= debugInfo){
		swSer.println("[Main] Heap free: "+String(freeheap));
		swSer.println("[Main] Size measure: "+String(sizeof(Measure)));
		swSer.println("[Main] measure_maxSize: "+String(measure_maxSize));
	}
	measures = new Measure[measure_maxSize];

	//init publish
	delay(3000);
	publishInit();

	//init clock
	timeClient.begin();
	timeClient.update();
	if(config.debugLevel >= debugInfo){
	  swSer.println("[Main] Time is: " + getDateTime(timeClient.getEpochTime()));
	}
  
	//wait and cancel reset
	delay(3000);
	if(config.debugLevel >= debugInfo){
		swSer.println("[Main] Init done. Stepping into mainloop.");
	}
	setWebConfigFlag(0);
}

void loop() {
	//true if whole telegram is received
	if(gatherData()){ 
		processData();
	}
	publishData();
  
	if(config.intervalMode == FixedOutputInterval){
		delay((config.intervalParameter * 1000) / 10);
	}
}
