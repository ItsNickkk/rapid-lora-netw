#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#define RH_HAVE_SERIAL
char buf[20];
uint8_t error;
RH_RF95 rf95; // Initializing RF95 driver
RHMesh *manager; // Initialize Manager to handle messages

void setup() {
    Serial.begin(115200);
    manager = new RHMesh(rf95, 2);
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
    buf[0] = '\0';
    strcat(buf, "1");
    error = manager->sendtoWait((uint8_t *)buf, strlen(buf), 3);
    delay(1000);
    buf[0] = '\0';
    strcat(buf, "0");
    error = manager->sendtoWait((uint8_t *)buf, strlen(buf), 3);
    delay(1000);
}
