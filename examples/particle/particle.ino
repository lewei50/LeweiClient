/*
   open.lewei50.com  sensor  client
 */

 #include <SPI.h>
 #include <Ethernet.h>
 #include <LeweiClient.h>
 #define USERKEY         "xxxxxxxxxxxx" // replace your key here


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
 double concentrationNofilter=0;
 
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
   // read the analog sensor:
   //int sensorReading = analogRead(A0);   

   // if there's incoming data from the net connection.
   // send it out the serial port.  This is for debugging
   // purposes only:
   
   if(1==sampling)
   {
     duration = pulseIn(pin, LOW);
     lowpulseoccupancy = lowpulseoccupancy+duration;

     if ((millis()-starttime) > sampletime_ms)
     {
       Serial.print("before filter:");
       Serial.println(lowpulseoccupancy);
       /*start:get the result without filter*/
       ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100       
       concentrationNofilter = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
       /*end*/    

       lowpulseoccupancy=Filter1(lowpulseoccupancy);
       
       //Serial.print("behind filter:");
       //Serial.println(lowpulseoccupancy);
       ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
       concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
       Serial.print(lowpulseoccupancy);
        Serial.print(",");
       Serial.print(ratio);
       Serial.print(",");
       Serial.println(concentration);
       lowpulseoccupancy = 0;
       //initiate the http post
       sampling=0;
       transfering=1;
     }
   }
   // http post begin 
   if(1==transfering)
   {
      lwc->append("p1", concentration);
      lwc->append("p2",concentrationNofilter);

     
        lwc->send();   
       Serial.print("leweiclient send:");    
       Serial.println(concentration);    
       transfering=0;
       sampling=1;
       starttime=millis();
   }
   

 }

 // this method makes a HTTP connection to the server:




