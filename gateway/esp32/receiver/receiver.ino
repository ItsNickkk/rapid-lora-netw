#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#define RH_HAVE_SERIAL
#define BASE_STATION_ID 1 //Change 
#include <ArduinoJson.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

uint8_t nodeID = BASE_STATION_ID;
char buf[RH_MESH_MAX_MESSAGE_LEN]; // Message Buffer

TaskHandle_t tskRecv;
TaskHandle_t tskPing;
RH_RF95 rf95; // Initializing RF95 driver
RHMesh *manager; // Initialize Manager to handle messages
StaticJsonDocument<200> JsonData;

void setup() {
	Serial.begin(115200);
	Serial.print(F("Initialing Node with ID: "));
	Serial.println(nodeID);

	manager = new RHMesh(rf95, nodeID); // Initialize the node in mesh networking layer
	if (!manager->init()) {
		Serial.println(F("Node not properly initalized."));
	} else {
		Serial.println("Node successfully initialized!");
	}
	Serial.println(F("Setting up RF95.."));

	rf95.setTxPower(23, false); // Setting the power output level of LoRa Module
	rf95.setFrequency(433.0); //Setting the frequency according to LoRa Module, in this case, 433mhz
	rf95.setCADTimeout(500);

	xTaskCreatePinnedToCore(tskRecvCode, "Receiving incoming messages", 10000, NULL, 1, &tskRecv, 1);
	xTaskCreatePinnedToCore(tskPingCode, "Ping", 10000, NULL, 1, &tskPing, 0);

	Serial.println(F("RF95 setup successful!"));
}

void loop() {

}
void tskRecvCode( void * pvParameters ) {
	Serial.print("Receiving Message code running on core ");
	Serial.println(xPortGetCoreID());
	for (;;) {
		TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
		TIMERG0.wdt_feed = 1;
		TIMERG0.wdt_wprotect = 0;
		uint8_t len = sizeof(buf);
		uint8_t from;
		if (manager->recvfromAckTimeout((uint8_t *)buf, &len, 2000, &from)) {
			buf[len] = '\0'; // null terminate string
			char head[10] = "Incoming=";
			char msg[265];
			msg[0] = '\0';
			strcat(msg, head);
			strcat(msg, buf);
			Serial.println(msg);
			RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
			//      if (route->next_hop != 0) {
			//          rssi[route->next_hop - 1] = rf95.lastRssi();
			//      }
		}
	}
}

void tskPingCode( void * pvParameters ) {
	Serial.print("Ping code running on core ");
	Serial.println(xPortGetCoreID());
	for (;;) {
		TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
		TIMERG0.wdt_feed = 1;
		TIMERG0.wdt_wprotect = 0;
//		if (Serial.available()) {
//			String temp = Serial.readString();
//			char tempBuf[255];
//			temp.toCharArray(tempBuf, sizeof(tempBuf));
//			DeserializationError error = deserializeJson(JsonData, tempBuf);
//			if (!error) {
//				String type = JsonData["action"];
//				if (type == "ping") {
//					int destination_node = (int)JsonData["dest"];
//					Serial.print("Ping Command to Node ");
//					Serial.println(destination_node);
//					char sendBuf[255] = "ping";
//					uint8_t error=99;
//					int i = 0;
//                  Serial.println(error == RH_ROUTER_ERROR_NONE);
//                  Serial.println(i);
//                  Serial.println(error != RH_ROUTER_ERROR_NONE && i < 5);
//					while (error != RH_ROUTER_ERROR_NONE && i < 5) {
//                        error = manager->sendtoWait((uint8_t *)sendBuf, strlen(sendBuf), destination_node);
//                        Serial.print("Pinging node ");
//                        Serial.println(destination_node);
//                        Serial.print("Try: ");
//                        Serial.println(i+1);
//                        i++;
//					}
//                    if(i>=4 && error != RH_ROUTER_ERROR_NONE){
//                        Serial.println("Incoming=PING_FAILED");
//                    }
//				}
//			}
//			else {
//				Serial.print(F("deserializeJson() failed: "));
//				Serial.println(error.f_str());
//			}
//
//		}
	}
}
