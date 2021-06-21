#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#define RH_HAVE_SERIAL
#define BASE_STATION_ID 1 //Change 

uint8_t nodeID = BASE_STATION_ID;
char buf[RH_MESH_MAX_MESSAGE_LEN]; // Message Buffer

RH_RF95 rf95; // Initializing RF95 driver
RHMesh *manager; // Initialize Manager to handle messages

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

	Serial.println(F("RF95 setup successful!"));
}

void loop() {
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
//		if (route->next_hop != 0) {
//			rssi[route->next_hop - 1] = rf95.lastRssi();
//		}
	}
}
