//############################################################
//			publishData
//############################################################

#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include "config.h"

WiFiClient espClient;
PubSubClient client;
HTTPClient http;

void callback(char* topic, byte* payload, unsigned int length) {
	/*
	 if (config.debugLevel >= debugInfo) {
	 swSer.print("Mqtt response: [");
	 swSer.print(topic);
	 swSer.println("] ");
	 for (int i = 0; i < length; i++) {
	 swSer.print((char) payload[i]);
	 }
	 swSer.println();
	 }*/
}

void publishInit() {
	if (config.mqtt_active) {
		client.setClient(espClient);
		client.setServer(config.mqtt_server.c_str(), config.mqtt_port);
		client.setCallback(callback);
		if (config.debugLevel >= debugInfo) {
			swSer.println("[PublishData] Set up mqtt");
		}
	}
}

void reconnect() {
	int failcount = 5;
	while (!client.connected() && failcount > 0) {
		if (client.connect("asdasdasd")) {
			if (config.debugLevel >= debugInfo) {
				swSer.println("[PublishData] Connected");
			}
			//client.publish(config.mqtt_topic.c_str(), "hello ");
			//client.subscribe(config.mqtt_topic.c_str());
		} else {
			if (config.debugLevel >= debugError) {
				swSer.print("[PublishData] Failed: ");
				swSer.print(client.state());
				swSer.println(", trying again in 5 seconds");
			}
			delay(5000);
			failcount--;
		}
	}
}
//right now: takes only one measure and posts them, while maybe 5 new measures are stored
void publishData() {
	int errorcount = 0;
	while (errorcount < PUBLISHMAXTRIES) {
		Measure* m;
		if (measure_size == 0 || (m = getNextMeasure()) == NULL) {
			if (config.debugLevel >= debugWarning) {
				swSer.println("[PublishData] Nothing to publish. Returning.");
			}
			return;
		}

		bool success = false;
		String label = config.patternLabel[m->patternId];
		String typeID = String(config.patternTypeID[m->patternId]);
		String datetime = getDateTime(m->timestamp);
		if (config.debugLevel >= debugInfo) {
			swSer.println("[PublishData]");
			swSer.println("ID       : " + String(m->id));
			swSer.println("PatternID: " + String(m->patternId));
			swSer.println("Label    : " + label);
			swSer.println("TypeID   : " + typeID);
			swSer.println("Value    : " + String(m->value));
			swSer.println("Timestamp: " + String(m->timestamp));
			swSer.println("Date     : " + datetime);
		}

		if (config.mqtt_active) {
			if (config.debugLevel >= debugInfo) {
				swSer.println("[PublishData] Publishing data over mqtt.");
			}
			if (!client.connected()) {
				reconnect();
			}
			client.loop();

			String mqttstr = datetime + ", ";
			if (config.mqtt_includeDeviceID) {
				mqttstr += config.deviceID + ", ";
			}
			if (config.mqtt_includeLabel) {
				mqttstr += label + ", ";
			}
			mqttstr += String(m->value);

			String topic;
			if (config.mqtt_useSubTopics) {
				topic = config.mqtt_topic + "/" + label;
			}
			else {
				topic = config.mqtt_topic;
			}
			if (config.debugLevel >= debugInfo) {
				swSer.println(topic + ": " + mqttstr);
			}
			success = client.publish(topic.c_str(), mqttstr.c_str());
		}

		if (config.rest_active) {
			if (config.debugLevel >= debugInfo) {
				swSer.println("[PublishData] Publishing data over rest.");
			}
			String reststr = String(
					"[{\"dateTime\":\"" + datetime + "\",\"value\":\""
					+ String(m->value) + "\",\"typeId\":" + typeID
					+ "}]");
			if (config.debugLevel >= debugInfo) {
				swSer.println(reststr);
			}
			http.begin(config.rest_server);
			http.addHeader("Content-Type", "application/json");
			int httpResponse = http.POST(reststr);
			success = ((httpResponse != -1) || success);

			String payload = http.getString();
			if (config.debugLevel >= debugInfo) {
				swSer.println(httpResponse);
				swSer.println(payload);
			}
			http.end();
		}

		if (success) {
			popMeasure();
		}
		else {
			errorcount++;
		}
	}
}
