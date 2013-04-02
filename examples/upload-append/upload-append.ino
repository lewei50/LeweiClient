#include <LeweiClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h> //BH1750 IIC Mode

#define LW_USERKEY "a1b1f96a9e92468e9067a5f41d525111"
#define LW_GATEWAY "01"

#define MY_NAME    "UNO1"
#define MY_DESC    "UNO test case"
char my_addr[50]="http://192.168.1.221/api";
int port =8889;
//delay between updates
LeWeiClient *lwc;

LeWeiAnalogSensor the_UVSensor("UV", "UV_sensor", "UVsensor", 0);

void setup() {
  String stringOne;
  Serial.begin(9600);

  uint8_t mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED  };
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

  // hope no exception here
  lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY,MY_NAME, MY_DESC, my_addr, (LeWeiClient::flag)((LeWeiClient::isControlled)|(LeWeiClient::internetAvailable)));
  // lwc = new LeWeiClient(LW_USERKEY, LW_GATEWAY,MY_NAME, MY_DESC, my_addr, (LeWeiClient::flag)(LeWeiClient::isControlled));

  lwc->registerSensor(the_UVSensor);

  Serial.print(lwc->nrSensors());
  Serial.println(F(" sensors registered."));
  Serial.print(lwc->nrActuators());
  Serial.println(F(" actuators registered."));

  lwc->initDevices();

  Serial.println(F("upload gateway info to server"));
  int retry = 10;
  while (lwc->uploadInfo() < 0 && --retry)
  {
    delay(500);

    Serial.print("retry: ");
    Serial.println(retry); 
  }
  if (!retry)
    Serial.println(F("uploadInfo failed in 10 times"));
  else
    Serial.println(F("uploadInfo done"));
}

void loop()
{
  static unsigned int loop_count;
  if (lwc)
  {
    loop_count++;

    Serial.print(F("*** loop nr: "));
    Serial.println(loop_count);
    lwc->append("INT", 112);
    lwc->append("DBL", -11.2238);
    lwc->scanSensors();
  }
  delay(500);
}

