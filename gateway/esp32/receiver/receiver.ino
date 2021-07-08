#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#define RH_HAVE_SERIAL
#define EEPROM_SIZE 1
#define BASE_STATION_ID 1 //Change 

char buf[RH_MESH_MAX_MESSAGE_LEN]; // Message Buffer

TaskHandle_t tskRecv;
TaskHandle_t tskPing;
RH_RF95 rf95; // Initializing RF95 driver
RHMesh *manager; // Initialize Manager to handle messages
StaticJsonDocument<200> JsonData;

void setup() {
	Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    uint8_t nodeID = EEPROM.read(0);
	Serial.print(F("Initialing Node with ID: "));
	Serial.println(nodeID);
    while (nodeID != BASE_STATION_ID){
        Serial.print("Expected ID: " + BASE_STATION_ID);
        Serial.print(" Actual Acquired ID: " + nodeID);
        Serial.println("The node ID does not matches the specified base station ID! Please verify these parameter according to the web portal!: ");
        Serial.println("1) ID in EEPROM (initNode.ino)");
        Serial.println("2) BASE_STATION_ID constant in receiver.ino");
        delay(1000);
    }

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
