#include <Wire_slave.h>
#include <i2cSimpleTransfer.h>
#include "C:/Users/Optiplex 9010/Documents/SolarProject/solarSystem/i2c/WorkingMapleSlave/MapleMiniVoltagesPercents/MapleMiniVoltagesPercents.ino"
/*
    https://github.com/getsurreal/i2cSimpleTransfer
*/

#define i2c_slave_address 17

/* Inverter */
// Charge Inverter
#define invPreCharge 19
// Run Inverter
#define invSolenoid 20
// Relay for inverter remote control
#define invPowerPin 22

/* Air Conditioner */
// Relay to control AC Output
#define acPin 21

#define ON LOW
#define OFF HIGH

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
bool inverterSleepStatus;
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

    /* Inverter */
    pinMode(invPreCharge, OUTPUT);
    pinMode(invSolenoid, OUTPUT);
    pinMode(invPowerPin, OUTPUT);
    digitalWrite(invPreCharge, OFF);
    digitalWrite(invSolenoid, OFF);
    digitalWrite(invPowerPin, OFF);

    /* Air Condiioner */
    pinMode(acPin,OUTPUT);
    digitalWrite(acPin, OFF);

    pinMode(bank1, INPUT_ANALOG);
    pinMode(bank2, INPUT_ANALOG);

}

void loop() {

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        displayVoltages(0,'v');
    }
    delay(10000);
    // Typical Day Demo
    /* Enable Inverter, Cycle(Enable AC, Disable AC, 
    sleep inverter), disable inverter*/
    Serial.println("First call of inverter");
    activateInverter();
    delay(5000);
    for(int i = 0; i < 5; i++){
        Serial.println(i);
        enableAC();
        delay(5000);
        deactivateAC();
        delay(5000);
        sleepInverter(true);
        delay(5000);
    }
    deactivateInverter();


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
// Helper function to flip switches after being called by enableAC
int activateAC(){
    if(inverter == true){
        // Check battery first, need 40% minimum each
        // if(checkBattery(1) > 40 && checkBattery(2) > 40){
        // POST ac on
        Serial.println("Enabling AC");
        digitalWrite(acPin,ON);
        ac = true;
        return 0;
    } else if(inverter == false){
        if(activateInverter() == 1 && inverter == true){
            digitalWrite(acPin,ON);
            ac = true;
        }
    }
}

int deactivateAC(){
    if(ac == false){
        // POST AC is already OFF
        return 0;
    } else if(ac == true){
        // POST turning OFF AC
        Serial.println("Turning AC Off");
        digitalWrite(acPin,OFF);
        ac = false;
        return 0;
    }
    return 1;
}

int enableAC(){
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
        return 0;
    } else if(ac == false){
        if(inverter == true){
            // POST AC on
            activateAC();
            return 0;
        } else {
            Serial.println("AC Called, inverter was off");
            activateInverter();
            if(inverter==true){
                // POST AC on
                activateAC();
                return 0;
            } else {
                // inverter failed to be on
                return 1;
            }
            return 1;
        }
    return 1;
    }
    return 1;

}

/* ===== Inverter Controls ===== */

// On in morning post sunrise, when voltage is good
// Off at night post or at sunset

int activateInverter(){
    Serial.println("Starting inverter");
    // Run PreCharge
    // Delay for balance
    // Run inverter invSolenoid
    preChargeComplete = false;
    Serial.println("\n\nTurning ON Pre Charge Circuit");
    digitalWrite(invPreCharge, ON);
    // POST precharge on
    delay(10000); // Wait for capacitors to charge
    preChargeComplete = true;
    // Only run if precharge has been done
    if(preChargeComplete == true){
        // POST inverter on
        Serial.println("Turning ON Inverter Main Solenoid Circuit");
        digitalWrite(invSolenoid, ON); // Enable Solenoid
        delay(5000); // delay for solenoid click
    Serial.println("Turning OFF Inverter Pre Charge Circuit\n\n");
    digitalWrite(invPreCharge,OFF); // Disable Precharge
    preChargeComplete = false;
    // POST precharge OFF
    inverter = true;
    return 0;
    }
    return 1;
}

int deactivateInverter(){
    if(ac == true){
        deactivateAC();
    }
    // POST inverter full power down
    Serial.println("Turning OFF Inverter Main Solenoid Circuit");
    digitalWrite(invSolenoid, OFF); // Disable Solenoid
    return 0;
}

int sleepInverter(bool sleep){
    if(inverterSleepStatus == sleep){
        Serial.println("Inverter is already asleep");
        return 0;
    }
    if(sleep == false && ac == false){
        // POST "inverter waking from sleep"
        Serial.println("Inverter wake from sleep");
        digitalWrite(invPowerPin, OFF); //Enable Low(off)
        inverterSleepStatus = false;
        return 0;
    } else if (sleep == true){
        if(ac == false){
            // POST "inverter going to sleep"
            Serial.println("Inverter going to sleep");
            digitalWrite(invPowerPin, ON); //Disable High(on)
            inverterSleepStatus = true;
            return 0;
        } else if (ac == true){
            // AC is on, we need to shut OFF first
            Serial.println("Inverter: AC was on");
            deactivateAC();
            sleepInverter(true);
            return 0;
        }
    }
    return 1;
}