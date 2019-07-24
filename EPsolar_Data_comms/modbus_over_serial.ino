#include <ModbusMaster.h>
#include <ESP8266WiFi.h>

extern char ssid[] ;
extern char pass[] ; 
extern const int debug = 1; //change to 0 when you are finished debugging
    
// instantiate ModbusMaster object
ModbusMaster node;

  // EIA 568B Cat 5e Cable wiring
  //White w/green B- & White w/blue A+ - Comms


// put something here if you want it to occur before or after transmission to the serial interface...i.e. delay etc. I have these blank and it works fine
void preTransmission()
{

}

void postTransmission()
{

}
/*
void setup()
{
  // Modbus communication runs at 115200 baud --> you can make this slower if you want, but works fine either way. I would suggest unplugging the RS485 from the ESP when you are uploading the code. Plug it back in as soon as the code successfully uploads
  Serial.begin(115200);
  // Modbus slave ID 1  --> this is the default for the 2210, so shouldn't need to change
  node.begin(1, Serial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  if (debug == 1){
    Serial.print("Connecting to WiFi.");
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (debug == 1){
      Serial.print(".");
    }
  }

  if (debug == 1){
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(5000);
  }

}

void loop(){
  readController();
} */

String getTime() {
  WiFiClient client;
  int numRetries = 0;
  while ((!!!client.connect("google.com", 80)) & (numRetries<10)) {
    Serial.println("connection failed, retrying...");
    if (WiFi.status() != WL_CONNECTED)
    {
      if (debug == 1){
        Serial.print("Reconnecting WiFi...");
      }
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED)
      {
        if (debug == 1){
          Serial.print(".");
        }
        delay(500);
      }
    }
    
    numRetries++;
  }
  
  if (numRetries==10){
    numRetries=0;
    while ((!!!client.connect("time.is", 80)) & (numRetries<10)) {
      Serial.println("connection failed, retrying...");
      if (WiFi.status() != WL_CONNECTED)
      {
        if (debug == 1){
          Serial.print("Reconnecting WiFi...");
        }
        WiFi.begin(ssid, pass);
        while (WiFi.status() != WL_CONNECTED)
        {
          if (debug == 1){
            Serial.print(".");
          }
          delay(500);
        }
      }
      
      numRetries++;
    }
  }

  client.print("HEAD / HTTP/1.1\r\n\r\n");
 
  while(!!!client.available()) {
     yield();
  }

  while(client.available()){
    if (client.read() == '\n') {    
      if (client.read() == 'D') {    
        if (client.read() == 'a') {    
          if (client.read() == 't') {    
            if (client.read() == 'e') {    
              if (client.read() == ':') {    
                client.read();
                String theDate = client.readStringUntil('\r');
                client.stop();
                return theDate;
              }
            }
          }
        }
      }
    }
  }
}

void updateTime(){
      String dtString;
      uint8_t result, dateDay, dateMonth, dateYear, timeHour, timeMinute, timeSecond;
      dtString = getTime();
      /*if (debug == 1){
        //Serial.println(dtString);
      }*/
      dateDay = atoi(dtString.substring(5,7).c_str());
      dateYear = atoi(dtString.substring(14,16).c_str());
      timeHour = atoi(dtString.substring(17,19).c_str());
      timeMinute = atoi(dtString.substring(20,22).c_str());
      timeSecond = atoi(dtString.substring(23,25).c_str());
      if (dtString.substring(8,11)=="Jan"){
          dateMonth=1;
      }
      if (dtString.substring(8,11)=="Feb"){
          dateMonth=2;
      }
      if (dtString.substring(8,11)=="Mar"){
          dateMonth=3;
      }
      if (dtString.substring(8,11)=="Apr"){
          dateMonth=4;
      }
      if (dtString.substring(8,11)=="May"){
          dateMonth=5;
      }
      if (dtString.substring(8,11)=="Jun"){
          dateMonth=6;
      }
      if (dtString.substring(8,11)=="Jul"){
          dateMonth=7;
      }
      if (dtString.substring(8,11)=="Aug"){
          dateMonth=8;
      }
      if (dtString.substring(8,11)=="Sep"){
          dateMonth=9;
      }
      if (dtString.substring(8,11)=="Oct"){
          dateMonth=10;
      }
      if (dtString.substring(8,11)=="Nov"){
          dateMonth=11;
      }
      if (dtString.substring(8,11)=="Dec"){
          dateMonth=12;
      }
    
      if (debug == 1){
        Serial.print("Date: ");
        Serial.print(dateDay);
        Serial.print("/");
        Serial.print(dateMonth);
        Serial.print("/");
        Serial.print(dateYear);
        Serial.print("     ");
        Serial.print(timeHour);
        Serial.print(":");
        Serial.print(timeMinute);
        Serial.print(":");
        Serial.println(timeSecond);
        //Serial.println(" ");
        //Serial.println(timeSecond<<8|timeMinute);
        //Serial.println(lowByte(timeSecond<<8|timeMinute));
  
        delay(2000);
      }
      node.setTransmitBuffer(0,(timeMinute<<8)|timeSecond);
      node.setTransmitBuffer(1,(dateDay<<8)|timeHour);
      node.setTransmitBuffer(2,(dateYear<<8)|dateMonth);
      result = node.writeMultipleRegisters(0x9013,3);
}

String readController()
{
  bool rs485DataReceived = true;
  int numLoops = 10000;

  uint8_t result,time1, time2, time3, date1, date2, date3;
  // uint16_t data[6];
  char buf[10];
  char bufDate[10];
  float bvoltage, ctemp, btemp, bremaining, lpower, lcurrent, pvvoltage, pvcurrent, pvpower;

  if (debug == 1){
    Serial.print("Beginning Loop ");
    Serial.println(numLoops);
  }

//Get Date and Time, and update Controller Data and Time  IN UTC, Day,Month,Year

  if (numLoops == 10000){ // Get date and time every 10000 loops

    updateTime();
      numLoops = 0;
      
  }   //End of get date and time

  delay(500);
  
  if (debug == 1){
    result = node.readHoldingRegisters(0x9013,3);
    if (result == node.ku8MBSuccess)
    {
      if (debug == 1){
        Serial.println("TimeDate--------------------------------------------------------------");
        Serial.print("Time is: ");
      }
      time1 = lowByte(node.getResponseBuffer(0x00));
      time2 = highByte(node.getResponseBuffer(0x00));
      time3 = lowByte(node.getResponseBuffer(0x01));
      
      sprintf(buf, "%02d:%02d:%02d",time3,time2,time1);
      Serial.println(buf);
  
      //Serial.println(time2<<8|time1);
      //Serial.println(node.getResponseBuffer(0x00));
      
      //Serial.println((time2<<8|time1)==(node.getResponseBuffer(0x00)));
      date1 = (node.getResponseBuffer(0x01) >>8);
      //Serial.println(time3);
      Serial.print("Date is: ");
      date2 = node.getResponseBuffer(0x02) & 0xff;
      date3 = (node.getResponseBuffer(0x02) >>8);
      //Serial.println(date2);
      //Serial.println(date3);
      sprintf(bufDate, "%02d/%02d/%02d",date2,date1,date3);
      Serial.println(bufDate);

      Serial.println("------------------------------------------------------------------");
    }else{
      rs485DataReceived = false;
    }
    delay(1000);  
  }  

  
  // Read 20 registers starting at 0x3100)
  result = node.readInputRegisters(0x3100, 20);
  if (result == node.ku8MBSuccess)
  {
    if (debug == 1){
      Serial.println("Controller Status---------------------------------------------------------");
      Serial.print("Controller Temperature: ");
    }
    ctemp = node.getResponseBuffer(0x11)/100.0f;
    if (debug == 1){
      Serial.println(ctemp);
      Serial.print("Battery Voltage: ");
    }
    bvoltage = node.getResponseBuffer(0x04)/100.0f; 
    if (debug == 1){
      Serial.println(bvoltage);
      Serial.print("Load Power: ");
    }
    lpower = ((long)node.getResponseBuffer(0x0F)<<16|node.getResponseBuffer(0x0E))/100.0f;
    if (debug == 1){
       Serial.println(lpower);    
       Serial.print("Load Current: ");
    }
    lcurrent = (long)node.getResponseBuffer(0x0D)/100.0f;
    if (debug == 1){
      Serial.println(lcurrent);
      Serial.print("PV Voltage: ");
    }
    pvvoltage = (long)node.getResponseBuffer(0x00)/100.0f;
    if (debug == 1){
      Serial.println(pvvoltage);
      Serial.print("PV Current: ");
    }
    pvcurrent = (long)node.getResponseBuffer(0x01)/100.0f;
    if (debug == 1){
      Serial.println(pvcurrent);
      Serial.print("PV Power: ");
    }
    pvpower = ((long)node.getResponseBuffer(0x03)<<16|node.getResponseBuffer(0x02))/100.0f;
    if (debug == 1){
      Serial.println(pvpower);
      Serial.println("------------------------------------------------------------------");
      delay(500);
    }
  }else{
    rs485DataReceived = false;
  }

  
  delay(500);
 
  result = node.readInputRegisters(0x311A, 2);
  if (result == node.ku8MBSuccess)
  {
    if (debug == 1){
      Serial.println("------------------------------------------------------------------");
      Serial.print("Battery Remaining %: ");
    }
    bremaining = node.getResponseBuffer(0x00)/1.0f;
    if (debug == 1){
      Serial.println(bremaining);
      Serial.print("Battery Temperature: ");
    }
    btemp = node.getResponseBuffer(0x01)/100.0f;
    if (debug == 1){
      Serial.println(btemp);
      Serial.println("------------------------------------------------------------------");
      delay(500);
    }
  }else{
    rs485DataReceived = false;
  }


  
  delay(500);

  if (rs485DataReceived){
     
      // field 1 - 8 is setup in thingspeak

      String url = "";
        url += "";
        url += bvoltage;
        url += ",";
        url += bremaining;
        url +=",";
        url += lcurrent;
        url += ",";
        url += lpower;
        url += ",";
        url += pvvoltage;
        url += ",";
        url += pvcurrent;
        url += ",";
        url += pvpower;  
        url += ",";
        url += ctemp;
        
        if (debug == 1){
          Serial.print("Requesting URL: ");
          //Serial.println(url);
        }
        
  return url;
  }
  numLoops++;
  
  delay(20000);
  
  if (WiFi.status() != WL_CONNECTED)
  {
    if (debug == 1){
      Serial.print("Reconnecting WiFi...");
    }
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
      if (debug == 1){
        Serial.print(".");
      }
      delay(500);
    }
  }
  
} //end Loop


