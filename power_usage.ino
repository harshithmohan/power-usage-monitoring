#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID   "PocoF1"
#define WLAN_PASS   "12345678"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "mohan226"
#define AIO_KEY         "9dd4854894a844ebafae467ae17ac854"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/current2");


const int sensorIn = A0;
int mVperAmp = 100; //185mV for 5A, 100mV for 10A, 66mV for 20A
double Vpp = 0;
double Vp = 0;
double Vrms = 0;
double Irms = 0;

void setup() { 
  Serial.begin(9600);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.println("Connecting...");
    delay(1000);
  }
  Serial.println("Connected");
}

void loop() {
  MQTT_connect();
  Vpp = getVPP();
  Vp = Vpp/2.0;
  Vrms = Vp*0.707; 
  Irms = ((Vrms * 1000)/mVperAmp) - 0.05;
  Serial.print(Irms);
  Serial.println(" Amps");
  if (! photocell.publish(Irms))
  {
    Serial.println("Failed");
  }
  else
  {
    Serial.println("OK!");
  }
  delay(2000);
}

double getVPP()
{
  double result;
  
  int readValue;
  int maxValue = 0;
  int minValue = 1024;
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           minValue = readValue;
       }
   }
   result = ((maxValue - minValue) * 5.0)/1024.0;
   return result;
 }

void MQTT_connect() {
  int8_t ret;
  
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
