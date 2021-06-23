#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#define RH_HAVE_SERIAL
char buf[20];
int command;
uint8_t error;
RH_RF95 rf95; // Initializing RF95 driver
RHMesh *manager; // Initialize Manager to handle messages

void setup() {
    Serial.begin(115200);
    pinMode(12, OUTPUT);
    manager = new RHMesh(rf95, 4);
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
//    buf[0] = '\0';
//    strcat(buf, "1");
//    error = manager->sendtoWait((uint8_t *)buf, strlen(buf), 4);
//    delay(1000);
//    buf[0] = '\0';
//    strcat(buf, "0");
//    error = manager->sendtoWait((uint8_t *)buf, strlen(buf), 4);
//    delay(1000);
//
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager->recvfromAckTimeout((uint8_t *)buf, &len, 2000, &from)) {
        buf[len] = '\0'; // null terminate string
        Serial.print("Received :");
        Serial.println(buf);
        command = atoi(&buf[0]);
        if(command == 1){
            digitalWrite(12, HIGH);
            Serial.println("HIGH");
        }
        else if(command == 0){
            digitalWrite(12, LOW);
            Serial.println("LOW");
        }
        RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
    }
}
