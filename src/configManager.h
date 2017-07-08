//############################################################
//			configManager
//Load config files from flash and provide webConfig
//functionality
//############################################################
#include <FS.h>
#include <ArduinoJson.h>
#include "webConfig.h"

bool shouldSaveConfig = false;
WebConfig WebConfig;

//callback notifying us of the need to save config
void saveConfigCallback() {
	shouldSaveConfig = true;
}

void logCallback(String s) {
	swSer.println();
	swSer.println(s);
	swSer.println();
}

//callback notifying us of the need to save config
void readTelegramCallback(int baud) {
	if (baud != config.baud) {
		config.baud = baud;
		Serial.end();
		Serial.begin(config.baud);
		if (config.debugLevel >= debugInfo) {
			swSer.println("[ConfigManager] Changing baudrate:");
			swSer.println(String(baud));
		}
	}

	inbuffer = "";
	long start = millis();
	while (!gatherData() && (millis() - start) < TELETIMELIMIT)
		;

	if ((millis() - start) > TELETIMELIMIT)
		config.webUITelegram =
				"Couldn't receive/parse telegram. Check Baudrate/Position of IR-Head.";
	else {
		config.webUITelegram = getFormatted(inbuffer);
	}
}

void setWebConfigFlag(int i) {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["webConfig"] = i;
	File dataFile = SPIFFS.open("/webCfg.json", "w");
	if (!dataFile) {
		//swSer.println("[ConfigManager] Failed to open webCfg.json for writing");
		return;
	}
	json.printTo(dataFile);
	dataFile.close();
}

int getWebConfigFlag() {
	if (SPIFFS.begin()) {
		if (SPIFFS.exists("/webCfg.json")) {
			File dataFile = SPIFFS.open("/webCfg.json", "r");
			if (dataFile) {
				//swSer.println("[ConfigManager] Successfully opened webCfg.json for reading");
				size_t size = dataFile.size();
				std::unique_ptr<char[]> buf(new char[size]);
				dataFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				if (json.success()) {
					return atoi(json["webConfig"]);
				}
			}
		}
		//swSer.println("[ConfigManager] Failed to open webCfg.json");
	}
	return 3;
}

void writeConfig() {
	setWebConfigFlag(0);
	if (config.debugLevel >= debugInfo) {
		swSer.println("[ConfigManager] Writing following config:");
		printCfg(&config, swSer);
	}
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	json["deviceID"] = config.deviceID;
	json["baud"] = config.baud;
	json["debugLevel"] = (int) config.debugLevel;
	json["intervalMode"] = (int) config.intervalMode;
	json["intervalParameter"] = config.intervalParameter;
	JsonArray& patternBegin = jsonBuffer.createArray();
	JsonArray& patternEnd = jsonBuffer.createArray();
	JsonArray& patternLabel = jsonBuffer.createArray();
	JsonArray& patternLength = jsonBuffer.createArray();
	JsonArray& patternOffset = jsonBuffer.createArray();
	JsonArray& patternTypeID = jsonBuffer.createArray();


	for (int i = 0; i < config.patternCount; i++) {
		patternEnd.add(config.patternEnd[i]);
		patternBegin.add(config.patternBegin[i]);
		patternLabel.add(config.patternLabel[i]);
		patternLength.add(config.patternLength[i]);
		patternOffset.add(config.patternOffset[i]);
		patternTypeID.add(config.patternTypeID[i]);
	}

	json["patternBegin"] = patternBegin;
	json["patternEnd"] = patternEnd;
	json["patternLabel"] = patternLabel;
	json["patternLength"] = patternLength;
	json["patternOffset"] = patternOffset;
	json["patternTypeID"] = patternTypeID;

	json["mqtt_active"] = config.mqtt_active ? 1 : 0;
	json["mqtt_server"] = config.mqtt_server;
	json["mqtt_port"] = config.mqtt_port;
	json["mqtt_topic"] = config.mqtt_topic;
	json["mqtt_useSubTopics"] = config.mqtt_useSubTopics ? 1 : 0;
	json["mqtt_includeDeviceID"] = config.mqtt_includeDeviceID ? 1 : 0;
	json["mqtt_includeLabel"] = config.mqtt_includeLabel ? 1 : 0;

	json["rest_active"] = config.rest_active ? 1 : 0;
	json["rest_server"] = config.rest_server;

	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
		if (config.debugLevel > debugOff) {
			swSer.println(
					"[configManager] Failed to open config file for writing.");
		}
	} else {
		json.printTo(configFile);
		configFile.close();
		if (config.debugLevel >= debugInfo) {

			swSer.println(
				"[configManager] Successfully opened config file for writing.");
			json.printTo(swSer);
			swSer.println();
			swSer.println("write finished");
		}
	}
	delay(1000);
}

void readConfig() {
	if (SPIFFS.exists("/config.json")) {
		if (config.debugLevel >= debugInfo) {
			swSer.println("[ConfigManager] Reading config file");
		}
		File configFile = SPIFFS.open("/config.json", "r");
		if (configFile) {
			if (config.debugLevel >= debugInfo) {
				swSer.println("[ConfigManager] Opened config file");
			}
			size_t size = configFile.size();
			std::unique_ptr<char[]> buf(new char[size]);
			configFile.readBytes(buf.get(), size);
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.parseObject(buf.get());

			if (json.success()) {
				if (config.debugLevel >= debugInfo) {
					json.printTo(swSer);
					swSer.println();
				}
				config.deviceID = json["deviceID"].asString();
				config.baud = atoi(json["baud"]);
				config.debugLevel = (DebugLevel) atoi(json["debugLevel"]);
				config.intervalMode = (ProcessMode) atoi(json["intervalMode"]);
				config.intervalParameter = atoi(json["intervalParameter"]);

				config.mqtt_active = (atoi(json["mqtt_active"]) == 1);
				config.mqtt_server = json["mqtt_server"].asString();
				config.mqtt_topic = json["mqtt_topic"].asString();
				config.mqtt_port = atoi(json["mqtt_port"]);
				config.mqtt_useSubTopics = json["mqtt_useSubTopics"];
				config.mqtt_includeDeviceID = json["mqtt_includeDeviceID"];
				config.mqtt_includeLabel = json["mqtt_includeLabel"];

				config.rest_active = (atoi(json["rest_active"]) == 1);
				config.rest_server = json["rest_server"].asString();

				JsonArray& patternBegin = json["patternBegin"];
				JsonArray& patternEnd = json["patternEnd"];
				JsonArray& patternLabel = json["patternLabel"];
				JsonArray& patternOffset = json["patternOffset"];
				JsonArray& patternLength = json["patternLength"];
				JsonArray& patternTypeID = json["patternTypeID"];
				config.patternCount = patternBegin.size();

				for (int i = 0; i < patternBegin.size(); i++) {
					config.patternBegin[i] = patternBegin[i].asString();
					config.patternEnd[i] = patternEnd[i].asString();
					config.patternLabel[i] = patternLabel[i].asString();
					config.patternTypeID[i] = patternTypeID[i];
					config.patternOffset[i] = patternOffset[i];
					config.patternLength[i] = patternLength[i];
				}
				if (config.debugLevel >= debugInfo) {
					swSer.println("[ConfigManager] Read in following config:");
					printCfg(&config, swSer);
				}
			} else {
				if (config.debugLevel > debugOff)
					swSer.println("[ConfigManager] Failed to load json config");
			}
		}
	}
}
void writeConfigCallback() {
	setWebConfigFlag(0);
	if (config.debugLevel >= debugInfo) {
		swSer.println("[ConfigManager] WriteConfig callback called.");
	}
	writeConfig();
}

void resetConfigCallback() {
	if (config.debugLevel >= debugInfo) {
    swSer.println("[ConfigManager] ResetConfig callback called.");
	}
  Config defaultCfg;
  config = defaultCfg;
  writeConfig();
  setWebConfigFlag(5);
}


void resetWebCounterCallback() {
  setWebConfigFlag(0);
}

void configManager(bool webConfig) {
	//clean FS, for testing	//SPIFFS.format();
	if (SPIFFS.begin()) {
		if (config.debugLevel >= debugInfo) {
			swSer.println("[ConfigManager] Mounted file system");
		}
		readConfig();
	} else {
		if (config.debugLevel > debugOff) {
			swSer.println("[ConfigManager] Failed to mount FS");
		}
	}

	WebConfig.setSaveConfigCallback(saveConfigCallback);
	WebConfig.setReadTelegramCallback(readTelegramCallback);
	WebConfig.setWriteConfigCallback(writeConfigCallback);
	WebConfig.setResetConfigCallback(resetConfigCallback);
	WebConfig.setResetWebCounterCallback(resetWebCounterCallback);
	WebConfig.setLogCallback(logCallback);

	if (webConfig) {
		if (config.debugLevel >= debugInfo) {
			swSer.println("[ConfigManager] WebConfig-flag is set");
		}
		WebConfig.resetSettings();
	}

	if (!WebConfig.autoConnect(APSSID.c_str(), APPASS.c_str())) {
		if (config.debugLevel > debugOff) {
			swSer.println("[ConfigManager] Failed to connect");
		}
		delay(3000);
		ESP.reset();
		delay(5000);
	}

	if (config.debugLevel >= debugInfo) {
		swSer.println("[ConfigManager] Connected");
	}

	if (shouldSaveConfig) {
		writeConfig();
	}

	if (config.debugLevel >= debugInfo) {
		swSer.print("[configManager] Local IP is ");
		swSer.println(WiFi.localIP());
	}
}
