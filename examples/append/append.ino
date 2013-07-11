/*
   open.lewei50.com  sensor  client
 */

 #include <SPI.h>
 #include <Ethernet.h>
 #include <LeweiClient.h>
 #define USERKEY         "xxx" // replace your key here


 #define LW_GATEWAY "01"
  
 
 LeWeiClient *lwc;


 unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
 boolean lastConnected = false;                 // state of the connection last time through the main loop
 const unsigned long postingInterval = 30*1000; //delay between updates to cosm.com


 int pin = 8;
 unsigned long duration;
 unsigned long starttime;
 unsigned long sampletime_ms = 30000;
 unsigned long lowpulseoccupancy = 0;
 float ratio = 0;
 double concentration = 0;

 void setup() {

   // start serial port:
   Serial.begin(9600);
   pinMode(8,INPUT);
   
   lwc = new LeWeiClient(USERKEY, LW_GATEWAY);

    starttime = millis();
 }
/* filter program : 20130521 */
#define FILTERLEN 10 

unsigned long Array_Average( unsigned long* Array,int length)
{
    int x;
    unsigned long returnVal;
    unsigned long result=0;
    for(x=0;x<length;x++)
    {
      result+=Array[x];
      Serial.print("result=");
      Serial.println(result);
    }
    returnVal=result/length;
    return returnVal;
}

unsigned long Filter1(unsigned long lowpulse)
{
  static unsigned long sfiterArray[FILTERLEN];
  static int sindex=0;
  int x;
   Serial.println("filter1 begin:");
  if(FILTERLEN>sindex)
  {
      sindex++;
      Serial.println(sindex);
      sfiterArray[sindex]=lowpulse;
         Serial.println("filter1 END");
      return lowpulse;
  }
  else
  {
      for(x=0;x<FILTERLEN-1;x++)
      {
        sfiterArray[x]=sfiterArray[x+1];
      }
      sfiterArray[FILTERLEN-1]=lowpulse;
      for(x=0;x<FILTERLEN;x++)
      {
         Serial.println(sfiterArray[x]);
      }
      Serial.println("Aver:");
       Serial.println(Array_Average(sfiterArray,FILTERLEN));
       Serial.println("filter1 END");
      return(Array_Average(sfiterArray,FILTERLEN));
      
  }
 

}
/*END: filter program : 20130521 */ 

 int x=0; //simulated sensor output
 int sampling=1;
 int transfering=0;
 void loop() {
 
 
      lwc->append("p1", 100.33);
      lwc->append("abc", 10000);
       lwc->append("def", 25.33); 
      
     
        lwc->send();   
        delay(5000);
 }

 // this method makes a HTTP connection to the server:





