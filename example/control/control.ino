#include <LeweiClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h> //BH1750 IIC Mode

#define LW_USERKEY "8xxxxxa4805"
#define LW_GATEWAY "01"

#define MY_NAME    "UNO1"
#define MY_DESC    "UNO test case"
char my_addr[50]="http://192.168.1.221/api";
int port =8889;
//delay between updates


LeWeiClient *lwc;


const int ledPin_da = 7;
const int ledPin_da2 = 6;




class dummy_act: public LeWeiActuator  
  
{  
  
   private:char led;   
  
   public:  
  
        dummy_act(  
  
                const char *id,  
  
                const char *type,   
  
                const char *name) : LeWeiActuator(id, type, name)  
  
    {};  
  
     
        virtual bool updateValue(char* id,int val)  
  
        {  
  
            Serial.print("dev id = ");   
            Serial.println(id);  
            if(!strcmp(id,"DA"))
            {
              Serial.println("handle da routin");  
              
              digitalWrite(ledPin_da, val);
            }
            if(!strcmp(id,"DA2"))
            {
              Serial.println("handle da2 routin");  
                  digitalWrite(ledPin_da2, val);
            }
            Serial.print("update actuator ");  
  
            Serial.println(val);  
  
            led=val;  
  
            return val;  
  
        }  
     
  
        virtual bool getValue(int *val)  
  
        {  
  
            static int i = 0;  
  
            *val = led;  
  
            return true;  
  
        }  
  
};  


dummy_act the_act("DA", "dummy actuator", "dummy-da");
dummy_act the_act2("DA2", "dummy actuator", "dummy-da2");

void setup() {
    String stringOne;
    Serial.begin(9600);

    uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
#if 0
    IPAddress  myip(192, 168, 1, 233);
    IPAddress  dnsip(211, 98, 2, 4);
    IPAddress  gateway(192, 168, 1, 1);
    Ethernet.begin(mac, myip, dnsip, gateway);
#else
    Serial.println("DHCP in process !"); 
    if (Ethernet.begin(mac) == 0)
    {
        Serial.print(F("Failed to configure Ethernet using DHCP\n"));
    }
    else
    {
        Serial.print("My IP address: ");
        for (byte thisByte = 0; thisByte < 4; thisByte++) {
       // print the value of each byte of the IP address:
        Serial.print(Ethernet.localIP()[thisByte], DEC);
        Serial.print("."); 
  }
        Serial.println();
        Serial.println(F("Ethernet configuration OK\n"));
        stringOne="http://";
        stringOne+=Ethernet.localIP()[0];
        stringOne+=".";
        stringOne+=Ethernet.localIP()[1];
        stringOne+=".";
        stringOne+=Ethernet.localIP()[2];
        stringOne+=".";
        stringOne+=Ethernet.localIP()[3];
         stringOne+=":";
        stringOne+=port;
        stringOne+="/api";    
        Serial.println(stringOne);
        stringOne.toCharArray(my_addr, 50); 

    }
#endif
    pinMode(ledPin_da, OUTPUT);
    
    digitalWrite(ledPin_da, LOW);
    pinMode(ledPin_da2, OUTPUT);
    
    digitalWrite(ledPin_da2, LOW);


    // hope no exception here
     lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY,MY_NAME, MY_DESC, my_addr, (LeWeiClient::flag)((LeWeiClient::isControlled)|(LeWeiClient::internetAvailable)));
   // lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY,MY_NAME, MY_DESC, my_addr, (LeWeiClient::flag)(LeWeiClient::isControlled));
    //lwc->registerSensor(the_PM1Sensor);

    lwc->registerActuator(the_act);
    lwc->registerActuator(the_act2);
    //lwc->registerSensor(the_MoistureSensor2);
    //lwc->registerSensor(the_MoistureSensor3);
    /*
     *lwc->registerSensor(the_hum);
     *lwc->registerSensor(the_tempr);
     *lwc->registerSensor(the_bh1750);
     */

    Serial.print(lwc->nrSensors());
    Serial.println(F(" sensors registered."));
    Serial.print(lwc->nrActuators());
    Serial.println(F(" actuators registered."));

    lwc->initDevices();

    lwc->beginServe(port);

    Serial.println(F("upload gateway info to server"));
    int retry = 20;
    while (lwc->uploadInfo() < 0 && --retry)
    {       
        Serial.print("retry: ");
        Serial.println(retry); 
        delay(500);       
    }
    if (!retry)
        Serial.println(F("uploadInfo failed in 10 times"));
    else
        Serial.println(F("uploadInfo done"));
}

void loop() {
    static unsigned int loop_count;
    if (lwc) {
        loop_count++;

        Serial.print(F("*** loop nr: "));
        Serial.println(loop_count);

        Serial.println(F("*** serve"));
        for (int i = 0; i < 100; i++) {
            lwc->serve();
            delay(100);
        }
    }

}


