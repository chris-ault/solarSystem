#include <Wire_slave.h>
#include <i2cSimpleTransfer.h>
#include "..\..\SolarProject\MapleMiniVoltagesPercents\MapleMiniVoltagesPercents.ino"
/*
    https://github.com/getsurreal/i2cSimpleTransfer
*/

#define i2c_slave_address 17

// Charge Inverter
#define invPreCharge 6
// Run Inverter
#define invSolenoid 7
// Relay to control AC Output
#define acPin 8
// Relay for inverter remote control
#define invPowerPin 9


// You can add more variables into the struct, but the default limit for transfer size in the Wire library is 32 bytes
struct SLAVE_DATA {
    // uint16_t maxvalue = 65501
    uint16_t value1;     // use specific declarations to ensure communication between 16bit and 32bit controllers
    uint16_t value2 = 1;
};

struct SLAVE_CONFIG {
    char val[];        // use specific declarations to ensure communication between 16bit and 32bit controllers
};

SLAVE_DATA slave_data;
SLAVE_CONFIG slave_config;

bool ac;
bool inverter;
bool preChargeComplete;
bool batteryStatus;
unsigned long previousMillis = 0;
const long interval = 1000;  

void setup() {
    // i2c communication
    Wire.begin(i2c_slave_address);
    Wire.onRequest(requestEvent); 
    Wire.onReceive(receiveEvent);
    // serial communication
    Serial.begin(115200);
    delay(200);

    String test = "test";
    test.toCharArray(slave_config.val,10);
    
    ac = false;
    inverter = false;
    preChargeComplete = false;
    batteryStatus = false;

    pinMode(acPin,OUTPUT);
    pinMode(invPreCharge, OUTPUT);
    pinMode(invSolenoid, OUTPUT);
    pinMode(bank1, INPUT_ANALOG);
    pinMode(bank2, INPUT_ANALOG);

}

void loop() {

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        displayVoltages();
    }

}

// Request, respond
void requestEvent() {
    Serial.print("Responding to request: ");    
    Serial.println("Sizeof " + String(slave_config.val) +  String(sizeof(slave_config.val)));
    Serial.print(String(slave_data.value1) + " " + String(slave_data.value2));
    i2cSimpleWrite(slave_data);            // Send the Master the sensor data
    slave_data.value1 += 1;  // Simulate updated sensor data
    // slave_data.value2 *= slave_config.val;
    Serial.println(" config:" + String(slave_config.val));
}

// Config Update Incoming
void receiveEvent (int payload) {
    Serial.print("Updating Config");
    if (payload > 0){
        i2cSimpleRead(slave_config);         // Receive new data from the Master
        Serial.println("New config:" + String(slave_config.val));
    }
}



/* ===== AC Controls ===== */
// On under good battery and good sun conditions 

int activateAC(){
    if(inverter == true){
        // Check battery first, need 40% minimum each
        // if(checkBattery(1) > 40 && checkBattery(2) > 40){
        // POST ac on
        digitalWrite(acPin,HIGH);
        ac = true;
    } else {
        if(activateInverter() == 1 && inverter == true){
            digitalWrite(acPin,HIGH);
            ac = true;
        }
    }
}

bool deactivateAC(){
    if(ac == false){
        return true;
    } else {
        // POST turning off AC
        digitalWrite(acPin,LOW);
        ac = false;
        return true;
    }
}

bool enableAC(){
    /*
    Check if AC is on
        -Yes
            Return 1
        -No
            Check if Inverter is on
                -Yes
                    Enable AC Relay
                    Return 1
                -No
                    Start Inverter

    */
    if(ac == true){
        return true;
    } else {
        if(inverter == true){
            activateAC();
        } else {
            if(activateInverter() == 1){
                // POST Inverter on
                activateAC();
                return true;
            } else {
                // inverter failed to return 1
                return false;
            }
        }
    }

}

/* ===== Inverter Controls ===== */
// On in morning
// Off at night

int activateInverter(){
    // Run PreCharge
    // Delay for balance
    // Run inverter invSolenoid
    preChargeComplete = false;
    digitalWrite(invPreCharge, HIGH);
    // POST precharge on
    delay(10000); // Wait for capacitors to charge
    preChargeComplete = true;
    if(preChargeComplete == true){
        // POST ac on
        digitalWrite(invSolenoid, HIGH); // Enable Solenoid
        delay(5000); // delay for solenoid click
    digitalWrite(invPreCharge,LOW); // Disable Precharge
    // POST precharge off
    }
    return 1;
}

int deactivateInverter(){

    return 1;
}

int sleepInverter(bool wake){
    if(wake == true){
        // POST "inverter waking from sleep"
        digitalWrite(invPowerPin, Low);
    } else if (wake == false){
        if(ac == false){
            // POST "inverter going to sleep"
            digitalWrite(invPowerPin, Low);
        } else if (ac == true){
            // AC is on, we need to shut off first
            deactivateAC();
        }
    }
}