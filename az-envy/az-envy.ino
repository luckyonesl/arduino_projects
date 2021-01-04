#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ConfigReader.h"
#include "utilities.h"
#include <MQ2.h>
#include <SHT3x.h>
#include "Ticker.h"

#define LED_BUILTIN 2

int without_wlan=0;

const String topics[] = {"/MQ-2/LPG", "/MQ-2/CO", "/MQ-2/Smoke", "/SHT30/Humidity", "/SHT30/Temperature"};


//used in setup and loop
String wifi_ssid;
String wifi_password;
String deviceName;
String mqtt_user;
String mqtt_password;
String MQTT_BROKER;
String TOPIC_ROOT;
int MQTT_PORT;
int pin = A0;
bool timer_is_attached=false;
WiFiClient wifiClient;
PubSubClient mqttclient(wifiClient);
SHT3x SensorSHT3x(0x44);
MQ2 SensorMQ2(pin);

void no_wifi_signal();
Ticker no_wifi_ticker_signal;

void led_signal()
{
  //Serial.println("led_signal");
  //led signals send
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(74880); // Aufbau einer seriellen Verbindung
  Serial.setTimeout(2000);
  while (!Serial) {};
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  ConfigReader CONFIG("/config.txt");
  //debugging via code  Serial.setDebugOutput(true);
  //read the config values
  wifi_ssid = CONFIG.getConfigValue("wifi_ssid");
  wifi_password = CONFIG.getConfigValue("wifi_password");
  deviceName = CONFIG.getConfigValue("deviceName");
  mqtt_user = CONFIG.getConfigValue("mqtt_user");
  mqtt_password = CONFIG.getConfigValue("mqtt_password");
  MQTT_BROKER = CONFIG.getConfigValue("MQTT_BROKER");
  MQTT_PORT = CONFIG.getConfigValue("MQTT_PORT").toInt();
  TOPIC_ROOT = CONFIG.getConfigValue("TOPIC_ROOT");
  
  wifi_establish_connection(wifi_ssid, wifi_password, deviceName);
  //1 mal alle 10 sec
  SensorSHT3x.SetUpdateInterval(10*1000);
  SensorSHT3x.Begin();
  SensorMQ2.begin();
  Serial.println("leave setup()");
}

// the loop function runs over and over again forever
void loop() {
  String topic;
  char buf[50];
  float* values;
  float  tmp_res;
  
  Serial.println(F("enter loop"));
  values=SensorMQ2.read(false);
  SensorSHT3x.UpdateData();
  
  if ( ! wifi_is_connected() )
  {
    if ( ! timer_is_attached )
    {
      Serial.println("attach timer");
      //no wifi start blinking
      no_wifi_ticker_signal.attach(1,led_signal);  
      timer_is_attached=true; 
      //disconnect the mqtt client
      mqttclient.disconnect();
    }
    wifi_teardown_rst(300,&without_wlan);
    Serial.print("in loop for ");Serial.print(without_wlan);Serial.println(" calls.");
    return;
  }
  else
  {
    if ( timer_is_attached == true )
    {
      no_wifi_ticker_signal.detach(); 
      timer_is_attached=false;
      delay(300);
      //switch of blinking
      digitalWrite(LED_BUILTIN, HIGH);  
    }
  }
  /*
  Serial.println("----------read SensorSHT3x------------");
  Serial.println(SensorSHT3x.GetTemperature(SHT3x::Cel));
  Serial.println(SensorSHT3x.GetRelHumidity());
  Serial.println("----------read SensorMQ2------------");
  Serial.println(SensorMQ2.readLPG());
  Serial.println(SensorMQ2.readCO());
  Serial.println(SensorMQ2.readSmoke());
  
  Serial.println(MQTT_BROKER);
  Serial.println(deviceName);
  Serial.println(mqtt_user);
  Serial.println(mqtt_password);*/
  mqttclient.setServer(MQTT_BROKER.c_str(), MQTT_PORT);
  if ( mqttclient.connect((char*)deviceName.c_str(), (char*)mqtt_user.c_str(), (char*)mqtt_password.c_str()) )
  {
    Serial.println("mqtt connect");
    //const String topics[] = {"/MQ-2/LPG", "/MQ-2/CO", "/MQ-2/Smoke", "/SHT30/Humidity", "/SHT30/Temperature"};
    for ( int i = 0 ; i < 5 ; i++)
    {
      topic = TOPIC_ROOT + "/" + deviceName + topics[i];
      switch ( i )
      { 
        case 0:
          dtostrf(SensorMQ2.readLPG(), 20, 2, buf);
          break;
        case 1:
          dtostrf(SensorMQ2.readCO(), 20, 2, buf);
          break; 
        case 2:
          dtostrf(SensorMQ2.readSmoke(), 20, 2, buf);
          break; 
        case 3:
          dtostrf(SensorSHT3x.GetRelHumidity(), 8, 2, buf);
          break; 
        case 4:
          //4 to much
          tmp_res=SensorSHT3x.GetTemperature(SHT3x::Cel);
          tmp_res+=-4;
          dtostrf(tmp_res, 8, 2, buf);
          break; 
      }
      mqttclient.publish(topic.c_str(), buf);
    }

  }
  mqttclient.disconnect();
  //only every 10 seconds
  delay(10000); 
}
