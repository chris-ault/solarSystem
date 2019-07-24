#include <Wire.h>
#include <i2cSimpleTransfer.h>
#include "loginCred.h"
#include "C:/Users/Optiplex 9010/Documents/SolarProject/solarSystem/EPsolar_Data_comms/modbus_over_serial.ino"
#define i2c_sensor_slave 17
/*
    https://github.com/getsurreal/i2cSimpleTransfer
*/

// May return odd sizing returns https://github.com/esp8266/Arduino/issues/1825
// Wemos library is using a 128 byte limit
struct SLAVE_DATA { // Can hold 32 x 4 byte floats(maple), only need 4 digits
    float voltageSolar;
    float voltageBatt;
    float currentSolar;
    float currentLoad;
    float powerSolar;
    float powerLoad;
    float temperatureCont;
    float temperatureBatt;
    float batremain;
};
/* Data fields
-------------------------------
    Controller Temperature
    
    Battery Voltage
    Battery Percent Remaining
    Battery Temperature
    
    Solar Voltage
    Solar Current
    Solar Power

    Load Power
    Load Current
*/


struct SLAVE_CONFIG {
    char val[3];
};

SLAVE_DATA slave_data;
SLAVE_CONFIG slave_config;

//const int debug = 1;

void sendRequest();
void updateConfig();

char requests[10][3] = {"V1", "V2", "P1", "P2", "VP", "PP"};

void setup() {
    Wire.begin();     // Begin i2c Master
    Serial.begin(115200);
    // Modbus communication runs at 115200 baud --> you can make this slower if you want, but works fine either way. I would suggest unplugging the RS485 from the ESP when you are uploading the code. Plug it back in as soon as the code successfully uploads

  // Modbus slave ID 1  --> this is the default for the 2210, so shouldn't need to change
  node.begin(1, Serial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, pass);
    if (debug == 1){
    Serial.print("Connecting to WiFi.");
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (debug == 1){
      Serial.print(".");
    }

}
}

void loop() {
    String result;
    result = readController();
    Serial.println(result);
    sendRequest();
    
    if (slave_data.value1 == 5) {
        
        strncpy(slave_config.val, "v1", 3);
        updateConfig();
    }

    delay(10000);     // Just for example use.  Use some sort of timed action for implementation
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