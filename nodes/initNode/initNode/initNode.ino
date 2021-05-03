#include <EEPROM.h>

//Insert ID of the node here
uint8_t id = 1;

void setup() {
    //Begin Serial Monitor
    Serial.begin(115200);
    
    //Assign Node ID to EEPROM address 0
    Serial.println("***Assigning Node ID***");
    EEPROM.write(0, id);
    Serial.print("Node ID assigned! Target Node ID: " + id);
    
    //Verify if Node ID is successfully inserted to address 0
    Serial.print("***Node ID Verification start***");
    uint8_t eepromid = EEPROM.read(0);
    Serial.println("Acquired Node ID in EEPROM: " + eepromid);
    
    if (id != eepromid) {
        Serial.println("An error occured when attempting to assign ID to this node!");
        Serial.println("Expected Value: " + id);
        Serial.println("Actual Value from EEPROM: " + eepromid);
    } 
    else {
        Serial.println("ID successfully written into EEPROM! Upload sketch from '../node/node.ino' to continue.");
    }
}

void loop() {

}
