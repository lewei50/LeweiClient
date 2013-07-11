/*
   lewei50 open platform sensor client
   This code is in the public domain.
  */
 
#include <LeweiClient.h>
 #include <SPI.h>
 #include <Ethernet.h>
 
#define LW_USERKEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
 #define LW_GATEWAY "01"
 
//delay between updates
 #define POST_INTERVAL (10*1000)
 
LeWeiClient *lwc;
 
const unsigned long postingInterval = 10*1000;
 
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
    Serial.print("*** start data collection ");
    lwc->append("BH", 123);
    lwc->append("DS1", 456);
    Serial.print("*** start either net send ");
    lwc->send();
    Serial.print("*** stop either net send ");
 
   delay(POST_INTERVAL);
   }
 }