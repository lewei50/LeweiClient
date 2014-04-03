/*
   Devicebit open platform sensor client
   This code is in the public domain.
  */
 
#include <LeweiClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SHT1x.h>

// Specify data and clock connections and instantiate SHT1x object
#define dataPin 4
#define clockPin 5
SHT1x sht1x(dataPin, clockPin);
 
//put your api key here,find it in lewei50.com->my account->account setting
#define LW_USERKEY "YOUR_API_KEY_HERE"
//put your gateway number here,01 as default
#define LW_GATEWAY "01"
 
//delay between updates
#define POST_INTERVAL (30*1000)
 
LeWeiClient *lwc;
 
 
void setup() {
   // start serial port:
   Serial.begin(9600);
   // hope no exception here
   lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY);
 }
 
void loop() {
   // read the analog sensor:
   //int sensorReading = analogRead(A0);  
 
  // if there's incoming data from the net connection.
   // send it out the serial port.  This is for debugging
   // purposes only:
 
  if (lwc) {
    Serial.println("read data ");
    
    float temp_c;
    float temp_f;
    float humidity;
    
    // Read values from the sensor
    temp_c = sht1x.readTemperatureC();
    temp_f = sht1x.readTemperatureF();
    humidity = sht1x.readHumidity();
    //t1,t2.. must using the same name setting on web server.
    lwc->append("temp", temp_c);
    lwc->append("hum", humidity);
    //Serial.print("*** data send ***");
    lwc->send();
    //Grammar changed by Wei&Anonymous ;)
    Serial.println("*** send completed ***");
 
   delay(POST_INTERVAL);
   }
 }
