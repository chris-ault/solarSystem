#include <Wire.h>
#include <i2cSimpleTransfer.h>
#include "loginCred.c"

#define i2c_sensor_slave 17

// You can add more variables into the struct, but the default limit for transfer size in the Wire library is 32 bytes
struct SLAVE_DATA {
    uint16_t value1;  // use specific declarations to ensure communication between 16bit and 32bit controllers
    uint16_t value2;
};

struct SLAVE_CONFIG {
    char val[3];     // use specific declarations to ensure communication between 16bit and 32bit controllers
};

SLAVE_DATA slave_data;
SLAVE_CONFIG slave_config;


void sendRequest();
void updateConfig();

char requests[10][3] = {"V1", "V2", "P1", "P2", "VP", "PP"};

void setup() {
    Wire.begin();     // Begin i2c Master
    Serial.begin(115200);
}

void loop() {

    sendRequest();
    
    if (slave_data.value1 == 5) {
        
        strncpy(slave_config.val, "v1", 3);
        updateConfig();
    }

    delay(100);     // Just for example use.  Use some sort of timed action for implementation
}

void sendRequest(){
    Wire.requestFrom( i2c_sensor_slave, sizeof(slave_data) );    // request data from the Slave device the size of our struct
    
    if ( Wire.available() == sizeof(slave_data) ) {
        i2cSimpleRead(slave_data);
    }

    Serial.println(String(slave_data.value1) + " " + String(slave_data.value2));
}

void updateConfig(){
    Serial.println("Updating config");
    Wire.beginTransmission(i2c_sensor_slave);
        i2cSimpleWrite(slave_config);
    Wire.endTransmission();
}