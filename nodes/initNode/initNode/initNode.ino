#include <EEPROM.h>
#define EEPROM_SIZE 1
#define EEPROM_ADDR 0

uint8_t id = 2; //Insert ID of the node here

void setup() {
    //Initializing Serial & EEPROM
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    
    //Assign Node ID to EEPROM address 0
    Serial.println("***Assigning Node ID***");
    EEPROM.write(EEPROM_ADDR, id);
    EEPROM.commit();
    Serial.print("Node ID assigned! Target Node ID: ");
    Serial.println(id);
    
    //Verify if Node ID is successfully inserted to address 0
    Serial.println("***Node ID Verification start***");
    uint8_t eepromid = EEPROM.read(EEPROM_ADDR);
    Serial.print("Acquired Node ID in EEPROM: ");
    Serial.println(eepromid);
    
    if (id != eepromid) {
        Serial.println("An error occured when attempting to assign ID to this node!");
        Serial.print("Expected Value: ");
        Serial.println(id);
        Serial.println("Actual Value from EEPROM: " + eepromid);
    } 
    else {
        Serial.println("ID successfully written into EEPROM!");
    }
}

void loop() {

}
