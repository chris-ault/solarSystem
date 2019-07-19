// Disable standard dev library when done, it costs 100bytes

#include "Statistic.h"

#define bank1 3
#define bank2 4
bool output1 = 1;
bool outputStats = 0;

Statistic myStats;

/*
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(bank1, INPUT_ANALOG);
  pinMode(bank2, INPUT_ANALOG);
  //myStats.clear(); //explicitly start clean

}

void loop() {

  displayVoltages();
  
}
*/



// Convert 12.6 Volts into 100ish percent
//https://arachnoid.com/polysolve/
double regress(double x) {
  double terms[] = {
    -4.5442101499211269e+004,
     2.0129820626735571e+004,
    -3.5592564768798634e+003,
     3.1399626638463803e+002,
    -1.3821074712720572e+001,
     2.4283948994133356e-001
};
  
  double t = 1;
  double r = 0;
  for (double c : terms) {
    r += c * t;
    t *= x;
  }
  if (r < 0) r = 0;
  return r;
}


// Analog read and average with a warmup
int ReadBank(int num){
  // num = bank
  myStats.clear(); //explicitly start clean
  int i;
  int sval = 0;
  int trash=22;
  int cycles = 900;
  int wait = 100;
  int reading = 0;
  // Warmup
  for (i = 0; i < 50; i++){
    trash = analogRead(num);
    delayMicroseconds(500);
  }
  // Read
  // For loop of cycles within for loop of wait
  // Keep track of best std for each 
  for (i = 0; i < cycles; i++){
    int reading = analogRead(num);
    myStats.add(reading);
    sval = sval + reading;
    delayMicroseconds(wait);
  }
  if(outputStats == 1){
    Serial.print("Statistics for Bank #"+String(num));
    Serial.print("\tCount: ");
    Serial.println(myStats.count()); // print values added
    Serial.print("Average: ");
    Serial.println(myStats.average(), 4); // get the average
    Serial.print("Std deviation: ");
    Serial.println(myStats.pop_stdev(), 4); // get the std deviation
    Serial.println("\n\n");
    delay(500);
  }
  sval = sval / cycles;
  return sval;
}

void displayVoltages(int cell, char type){
  //cell: 0 is both banks result, 1 is bank 1, 2 is bank 2
  //type: a for analog, v for voltage, p for percent

  if(cell == 1){
    
  }

    // read the adcs
  int sensorValue1 = ReadBank(bank1);
  int sensorValue2 = ReadBank(bank2);
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage1 = sensorValue1 * (3.34 / 4095.0) ;
  float voltage2 = sensorValue2 * (3.27 / 4095.0);

  // Generate outputs
  String VoltagesR = String(voltage1);
  VoltagesR += ",\t\t";
  VoltagesR +=  String(voltage2);
  voltage1 = voltage1 * 3.81818;
  voltage2 = voltage2 * 3.81818;
  
  String Voltages = String(voltage1);
  Voltages += ",\t\t";
  Voltages +=  String(voltage2);
  String VoltageSum = String(voltage1+voltage2);
  Voltages += ",\t\t" + VoltageSum + "V";
  
  double b1percent = regress(voltage1);
  double b2percent = regress(voltage2);
  String Percents = String(b1percent*100) + "%";
  Percents += ",\t\t";
  Percents += String(b2percent*100) + "%";
  Percents += ",\t\t" + String((b1percent + b2percent)/2*100) + "% Avg.";

  // Print Results
  if(output1 == 1){
    Serial.println("\t\tBank 1\t\tBank2\t\tTotal");
    Serial.println("ADC:\t\t" + VoltagesR);
    Serial.println("Pack V:\t\t" + Voltages);
    Serial.println("Pack %:\t\t" +Percents);
    Serial.println("\n\n\n");
    delay(10);
  }
}
