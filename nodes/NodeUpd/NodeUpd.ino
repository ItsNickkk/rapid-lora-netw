// Temporary mobile ad hoc network infrastructure for rapid deployment during disaster relief by Chan Xin Yen TP046062

#include <EEPROM.h>
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#define RH_HAVE_SERIAL
#define EEPROM_SIZE 1
#define RH_TEST_NETWORK 3
// Line above can simulate condition where two of the nodes (address 2 & address 3) does not directly communicate with each other
// This network looks like 1-2-4
//                         |   |
//                         --3--

#define N_NODES 4 //Change the number of nodes in this variable

uint8_t nodeID;
uint8_t routes[N_NODES]; // Array containing routing info
int16_t rssi[N_NODES]; // Array containing RSSI (Received Signal Strength Indicator)
char buf[RH_MESH_MAX_MESSAGE_LEN]; // Message Buffer
const byte DNS_PORT = 53;

TaskHandle_t Task1;
TaskHandle_t Task2;
IPAddress apIP(172, 217, 28, 1); // The default android DNS
DNSServer dnsServer;
WiFiServer server(80);
RH_RF95 rf95; // Initializing RF95 driver
RHMesh *manager; // Initialize Manager to handle messages

String responseHTML = ""
"<!DOCTYPE html><html><head><title>CaptivePortal</title></head><body>"
"<h1>Hello World!</h1><p>This is a captive portal example. All requests will "
"be redirected here.</p></body></html>";

int freeMem() { // clear memory
return ESP.getFreeHeap();
}

void setup() {
    WiFi.mode(WIFI_AP);
WiFi.softAP("ESP32-DNSServer");
WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

// if DNSServer is started with "*" for domain name, it will reply with
// provided IP to all DNS request
dnsServer.start(DNS_PORT, "*", apIP);

server.begin();
Serial.begin(115200);
EEPROM.begin(EEPROM_SIZE);
nodeID = EEPROM.read(0);// Read node ID from EEPROM address 0, which is placed in using initNode.ino
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
xTaskCreatePinnedToCore(
             Task1code, /* Task function. */
             "Task1",   /* name of task. */
             10000,     /* Stack size of task */
             NULL,      /* parameter of the task */
             1,         /* priority of the task */
             &Task1,    /* Task handle to keep track of created task */
             0);        /* pin task to core 0 */
xTaskCreatePinnedToCore(
             Task2code,  /* Task function. */
             "Task2",    /* name of task. */
             10000,      /* Stack size of task */
             NULL,       /* parameter of the task */
             1,          /* priority of the task */
             &Task2,     /* Task handle to keep track of created task */
             1);         /* pin task to core 0 */
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
    if (n-1 == nodeID) {
        routes[n] = 255; // self
    }
    else {
        routes[n-1] = 0; //no route
    }
    nodeID = EEPROM.read(0);
    continue;
    }
    else {
    routes[n-1] = route->next_hop;
    if (routes[n-1] == 0) {
        // if we have no route to the node, reset the received signal strength
        rssi[n-1] = 0;
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

void buildbuf(char *p, size_t len) {  // Create a JSON string from buffer containing route info
    p[0] = '\0';
    strcat(p, "dsa");
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

void routeDiscover(){
    
    for (uint8_t n = 1; n <= N_NODES; n++) {
        if (n == nodeID) continue; // Ignore this loop if this is self loop

        updateRoutingTable();
        getRouteInfoString(buf, RH_MESH_MAX_MESSAGE_LEN);

        Serial.print(F("TO "));
        Serial.print(n);
        Serial.print(F(":"));
        Serial.println(buf);

        uint8_t error = manager->sendtoWait((uint8_t *)buf, strlen(buf), n); //Broadcast message to other nearby nodes & cast the return into error message
        
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
        unsigned long nextTransmit = millis() + random(3000, 5000);
        while (nextTransmit > millis()) {
            int waitTime = nextTransmit - millis();
            uint8_t len = sizeof(buf);
            uint8_t from;
            if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from)) {
                buf[len] = '\0'; // null terminate string
                Serial.print("FROM ");
                Serial.print(from);
                Serial.print(":");
                Serial.println(buf);
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

void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
TIMERG0.wdt_feed=1;
TIMERG0.wdt_wprotect=0;
    dnsServer.processNextRequest();
    WiFiClient client = server.available();   // listen for incoming clients

    if (client) {
        String currentLine = "";
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        client.print(responseHTML);
                        break;
                    } 
                    else {
                        currentLine = "";
                    }
                } 
                else if (c != '\r') {
                currentLine += c;
                }
            }
        }
        client.stop();
    }
  }
}

void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
TIMERG0.wdt_feed=1;
TIMERG0.wdt_wprotect=0;
    unsigned long nextTransmit = millis() + 2000;
    while (nextTransmit > millis()) {
        buildbuf(buf, RH_MESH_MAX_MESSAGE_LEN);
        uint8_t error = manager->sendtoWait((uint8_t *)buf, strlen(buf), 1);
        Serial.println(error);

    }
  }
}

void loop() {
    
}
