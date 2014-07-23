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
 #define POST_INTERVAL (30*1000)
 

//IPAddress ip(192,168,1, 15);
//IPAddress mydns(8,8,8,8);
//IPAddress gw(192,168,1,1);
//IPAddress subnet(255,255,255,0);
 
LeWeiClient *lwc;
 
 
void setup() {
   // start serial port:
   Serial.begin(9600);
   // hope no exception here
   lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY);
   //lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY,ip,mydns,gw,subnet);
 }
 
void loop() {
   // read the analog sensor:
   //int sensorReading = analogRead(A0);  
 
  // if there's incoming data from the net connection.
   // send it out the serial port.  This is for debugging
   // purposes only:
 
  if (lwc) {
    Serial.print("*** start data collection ");
    
    //t1,t2.. must using the same name setting on web server.
    lwc->append("t1", 123);
    lwc->append("t2", 456);
    Serial.print("*** data send ***");
    lwc->send();
    //Grammar changed by Wei&Anonymous ;)
    Serial.print("*** send completed ***");
 
   delay(POST_INTERVAL);
   }
 }
