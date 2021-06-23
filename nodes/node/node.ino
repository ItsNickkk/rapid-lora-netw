// Temporary mobile ad hoc network infrastructure for rapid deployment during disaster relief by Chan Xin Yen TP046062
// Please make sure you flashed Node ID using initNode.ino & flashed web portal to SPIFFS!!
// Make sure "N_NODES" constant indicates the correct amount of Nodes in the network
// Also make sure "*params" contains the correct parameters as your HTML form input's "name" attribute!

#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#include <EEPROM.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#define RH_HAVE_SERIAL
#define EEPROM_SIZE 1
#define N_NODES 4 //Change the number of nodes in this variable
#define BASE_STATION_ID 1 //Change 
#define STAT_LED 12

const char *params[] = {"n", "add", "hp", "cnt", "dg", "wt", "fd", "hg"};
//change the parameters based on what is sent from HTML form
//Keep the parameter names as short as possible and take note of LoRa max packet size (255)!
unsigned long nextLoRaTransmit = 0;
uint8_t nodeID;
uint8_t routes[N_NODES]; // Array containing routing info
int16_t rssi[N_NODES]; // Array containing RSSI (Received Signal Strength Indicator)
char buf[RH_MESH_MAX_MESSAGE_LEN]; // Message Buffer
char recvBuf[RH_MESH_MAX_MESSAGE_LEN]; // Message Buffer for received message

TaskHandle_t tskCaptive;
TaskHandle_t tskLora;
QueueHandle_t queueMsg;
IPAddress apIP(172, 217, 28, 1);
DNSServer dnsServer;
AsyncWebServer server(80);
RH_RF95 rf95; // Initializing RF95 driver
RHMesh *manager; // Initialize Manager to handle messages


class CaptiveRequestHandler : public AsyncWebHandler {
	public:
		CaptiveRequestHandler() {
			server.on("/get", HTTP_GET, [](AsyncWebServerRequest * request) {
				if (request -> params() > 0) {
					char getBuf[RH_MESH_MAX_MESSAGE_LEN];
					size_t form_param_size = sizeof(params) / sizeof(params[0]);
					getBuf[0] = '\0';
					strcat(getBuf, "{");
					for (int i = 0; i < form_param_size; i++) {
						if (request->hasParam(F(params[i]))) {
							AsyncWebParameter* p = request->getParam(F(params[i]));
							strcat(getBuf, "\"");
							strcat(getBuf, p->name().c_str());
							strcat(getBuf, "\":\"");
							strcat(getBuf, p->value().c_str());
							strcat(getBuf, "\",");
						}
					}
					strcat(getBuf, "\"node\":");
					sprintf(getBuf + strlen(getBuf), "%d", nodeID);
					strcat(getBuf, "}");
					Serial.println(getBuf);
					xQueueSend(queueMsg, &getBuf, 0);
					AsyncResponseStream *response = request->beginResponseStream("text/html");
					response->print("<!DOCTYPE html><html><head><title>Unauthorized Access</title></head><body>");
					response->print("<h1>Message successfully sent to base station! </h1>");
					response->print("<p>If you wish to update your condition, please disconnect and reconnect to this access point and resubmit the form!");
					response->print("</body></html>");
					request->send(response);
				}
				else {
					AsyncResponseStream *response = request->beginResponseStream("text/html");
					response->print("<!DOCTYPE html><html><head><title>Unauthorized Access</title></head><body>");
					response->print("<h1>Unauthorized Access</h1>");
					response->print("</body></html>");
					request->send(response);
				}
			});
			server.on("/src/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
				request->send(SPIFFS, "/src/bootstrap.bundle.min.js", "text/javascript");
			});

			server.on("/src/jquery-3.3.1.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
				request->send(SPIFFS, "/src/jquery-3.3.1.min.js", "text/javascript");
			});

			server.on("/src/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest * request) {
				request->send(SPIFFS, "/src/bootstrap.min.css", "text/css");
			});

			server.on("/src/main.css", HTTP_GET, [](AsyncWebServerRequest * request) {
				request->send(SPIFFS, "/src/main.css", "text/css");
			});

			server.on("/static/bg.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
				request->send(SPIFFS, "/static/bg.svg", "image/svg+xml");
			});
		}
		virtual ~CaptiveRequestHandler() {}

		bool canHandle(AsyncWebServerRequest *request) {
			return true;
		}

		void handleRequest(AsyncWebServerRequest *request) {
			//request->send_P(200, "text/html", index_html);
			request->send(SPIFFS, "/index.html", "text/html");
		}
};

void setup() {
	Serial.begin(115200);
	EEPROM.begin(EEPROM_SIZE);
	nodeID = EEPROM.read(0);// Read node ID from EEPROM address 0, which is placed in using initNode.ino
	if (!SPIFFS.begin()) {
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}
	pinMode(STAT_LED, OUTPUT);
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

	for (uint8_t n = 0; n < N_NODES; n++) {
		routes[n] = 0;
		rssi[n] = 0;
	}
	routeDiscover();
	queueMsg = xQueueCreate(10, RH_MESH_MAX_MESSAGE_LEN);
	xTaskCreatePinnedToCore(tskCaptiveCode, "Captive Portal", 10000, NULL, 1, &tskCaptive, 1);
	xTaskCreatePinnedToCore(tskLoraCode, "LoRa comm", 10000, NULL, 1, &tskLora, 0);
}

const __FlashStringHelper* getErrorString(uint8_t error) {
	switch (error) {
		case 1: return F("Invalid Length");
			break;
		case 2: return F("No route to node");
			break;
		case 3: return F("Timeout");
			break;
		case 4: return F("No reply");
			break;
		case 5: return F("Unable to Deliver");
			break;
	}
	return F("Unknown Error");
}

void updateRoutingTable() {
	for (uint8_t n = 1; n <= N_NODES; n++) {
		RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
		if (route == NULL) {
			if (n - 1 == nodeID) {
				routes[n] = 255; // self
			}
			else {
				routes[n - 1] = 0; //no route
			}
			nodeID = EEPROM.read(0);
			continue;
		}
		else {
			routes[n - 1] = route->next_hop;
			if (routes[n - 1] == 0) {
				// if we have no route to the node, reset the received signal strength
				rssi[n - 1] = 0;
			}
		}
	}
}


void getRouteInfoString(char *p, size_t len) {  // Create a JSON string from buffer containing route info
	p[0] = '\0';
	strcat(p, "[");
	for (uint8_t n = 0; n < N_NODES; n++) {
		strcat(p, "{\"n\":");
		sprintf(p + strlen(p), "%d", routes[n]);
		strcat(p, ",");
		strcat(p, "\"r\":");
		sprintf(p + strlen(p), "%d", rssi[n]);
		strcat(p, "}");
		if (n < N_NODES) {
			strcat(p, ",");
		}
	}
	strcat(p, "]");
}

void toggleStatus(bool cond) {
	digitalWrite(STAT_LED, cond);
}

void printNodeInfo(uint8_t node, char *s) {
	Serial.print(F("node: "));
	Serial.print(F("{"));
	Serial.print(F("\""));
	Serial.print(node);
	Serial.print(F("\""));
	Serial.print(F(": "));
	Serial.print(s);
	Serial.println(F("}"));
}

void routeDiscover() {

	for (uint8_t n = 1; n <= N_NODES; n++) {
		if (n == nodeID) continue; // Ignore this loop if this is self loop

		updateRoutingTable();
		getRouteInfoString(buf, RH_MESH_MAX_MESSAGE_LEN);

		Serial.print(F("TO "));
		Serial.print(n);
		Serial.print(F(":"));
		Serial.println(buf);
		uint8_t error;
		if (n == BASE_STATION_ID) {
			toggleStatus(true);
			while (error != RH_ROUTER_ERROR_NONE) {
				error = manager->sendtoWait((uint8_t *)buf, strlen(buf), n);
				Serial.println("Attempting to communicate with base station");
				delay(500);
			}
			Serial.println("Connection Established with base station");
			toggleStatus(false);
		}
		else {
			error = manager->sendtoWait((uint8_t *)buf, strlen(buf), n);
		}
		Serial.print(F("Send to node "));
		Serial.print(n);
		Serial.print(F(", "));
		if (error != RH_ROUTER_ERROR_NONE) {
			Serial.print(F("Err "));
			Serial.print(error);
			Serial.print(F(": "));
			Serial.println(getErrorString(error));
			Serial.println();
		}
		else {
			Serial.println(F("OK"));
			Serial.println();
			RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
			if (route->next_hop != 0) {
				rssi[route->next_hop - 1] = rf95.lastRssi();
			}
		}
		unsigned long nextTransmit = millis() + 1000;
		while (nextTransmit > millis()) {
			int waitTime = nextTransmit - millis();
			uint8_t len = sizeof(buf);
			uint8_t from;
			if (manager->recvfromAckTimeout((uint8_t *)recvBuf, &len, waitTime, &from)) {
				recvBuf[len] = '\0'; // null terminate string
				Serial.print("FROM ");
				Serial.print(":");
				Serial.println(recvBuf);
				if (nodeID == 1) printNodeInfo(from, buf); // debugging
				// we received data from node 'from', but it may have actually come from an intermediate node
				RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
				if (route->next_hop != 0) {
					rssi[route->next_hop - 1] = rf95.lastRssi();
				}
			}
		}
	}
}


void tskCaptiveCode( void * pvParameters ) {
	const char *ssid = "\xE2\x80\xBC Emergency Portal \xF0\x9F\x86\x98";
	Serial.println(ssid);
	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid);
	WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
	dnsServer.start(53, "*", apIP);
	server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
	server.begin();

	Serial.print("Webserver running on core ");
	Serial.println(xPortGetCoreID());
	for (;;) {
		TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
		TIMERG0.wdt_feed = 1;
		TIMERG0.wdt_wprotect = 0;
		dnsServer.processNextRequest();
	}
}


void tskLoraCode( void * pvParameters ) {
	Serial.print("LoRa transmit running on core ");
	Serial.println(xPortGetCoreID());

	for (;;) {
		TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
		TIMERG0.wdt_feed = 1;
		TIMERG0.wdt_wprotect = 0;
		if (millis() > nextLoRaTransmit && uxQueueMessagesWaiting(queueMsg) > 0) {
			xQueueReceive(queueMsg, &buf, 0);
			uint8_t error;
			toggleStatus(true);
			while (error != RH_ROUTER_ERROR_NONE) {
				error = manager->sendtoWait((uint8_t *)buf, strlen(buf), BASE_STATION_ID);
				Serial.println("Attempting to communicate with base station");
			}
			toggleStatus(false);
			nextLoRaTransmit = millis() + 500;
		}
		uint8_t len = sizeof(recvBuf);
		uint8_t from;
		if (manager->recvfromAckTimeout((uint8_t *)recvBuf, &len, 2000, &from)) {
			recvBuf[len] = '\0'; // null terminate string
//			Serial.println(recvBuf);
//            Serial.println(from);
//			if(from == 1 && recvBuf == "ping"){
//                char tempBuf[20];
//                tempBuf[0] = '\0';
//                strcat(tempBuf, "pong from ");
//                sprintf(tempBuf+strlen(tempBuf),"%d" , nodeID);
//                manager->sendtoWait((uint8_t *)tempBuf, strlen(buf), from);
//                Serial.println(tempBuf);
//			}
//			
			RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
			if (route->next_hop != 0) {
				rssi[route->next_hop - 1] = rf95.lastRssi();
			}
		}
	}
}

void loop() {

}
