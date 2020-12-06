
#include <Arduino.h>
#include "ConfigReader.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RTCVars.h>
#include <Ticker.h>
#include "BlueDot_BME280.h"

ConfigReader CONFIG("/config.txt");

BlueDot_BME280 bme280 = BlueDot_BME280();
RTCVars state; // create the state object

char buf[50];
unsigned int raw = 0;
float volt = 0.0;

String wifi_ssid;
String wifi_password;
String deviceName;
String mqtt_user;
String mqtt_password;
String MQTT_BROKER;
String TOPIC_ROOT;
int MQTT_PORT;

int channel;  // 1 byte,   5 in total
byte bssid[6]; // 6 bytes, 11 in total


const String topics[] = {"/BMP280/Voltage", "/BMP280/Temperature", "/BMP280/Humidity", "/BMP280/Pressure", "/BMP280/AltitudeMeter", "/BMP280/DewPoint"};

/*
  topics
  0->Voltage
  1->Temp
  2->Humidity
  3->Pressure
  4->AltitudeMeter
  5->DewPoint
*/

WiFiClient wifiClient;
Ticker flipper;
long blink_count;

float calc_dewpoint(unsigned int h , int t)
{
  float H, dew_point;
  H = (log10(h) - 2.0) / 0.4343 + (17.62 * t) / (243.12 + t);
  dew_point = 243.12 * H / (17.62 - H);
  return dew_point;
}

int get_powerstate() {
  int powerstate;
  //0 no save 1 switch off wifi 2 sleep 3 dep stande by
  //pin is set
  raw = analogRead(A0);
  volt = raw / 1023.0;
  volt = volt * 4.2;
  Serial.println(String(volt));
  if ( volt > 3.6 )
  {
    powerstate = 0;
    Serial.println(" online");
  }
  if ( volt <= 3.6 && volt > 3.4 )
  {
    powerstate = 1;
    Serial.println(" online but switch of wifi");
  }
  if ( volt <= 3.4 )
  {
    powerstate = 2;
    Serial.println("deep sleep max");
  }
  //3 is missing
  return powerstate;
}

void connect_wifi() {
  int count_round = 0;
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.hostname(deviceName);
    //WiFi.config(staticIP, subnet, gateway, dns);
    if ( bssid[0] == 0 )
    {
      Serial.println("use ssid / password ");
      WiFi.begin(wifi_ssid, wifi_password);
    }
    else
    {
      Serial.println("use ssid / password / channel / bssid");
      WiFi.begin(wifi_ssid, wifi_password, channel, bssid, true);
    }
    WiFi.mode(WIFI_STA);
    wifi_set_sleep_type(NONE_SLEEP_T);
  }
  while (WiFi.status() != WL_CONNECTED && count_round < 100)
  {
    count_round++;
    if ( count_round > 99 )
    {
      bssid[0] = 0;
      Serial.println("start with a network scan");
      WiFi.disconnect();
    }
    delay(500);
    Serial.println("wait for wifi");
  }
  if ( bssid[0] == 0 && WiFi.status() == WL_CONNECTED )
  {
    Serial.println("copy bssid and channel once to RTC");
    memcpy( bssid, WiFi.BSSID(), 6 );
    channel = WiFi.channel();
  }
  Serial.println(WiFi.localIP());
}

PubSubClient mqttclient(wifiClient);

void connect_mqtt() {
//  mqttclient;
//(MQTT_BROKER.c_str(), MQTT_PORT, callback, wifiClient);
  int count_round = 0;
  mqttclient.setServer(MQTT_BROKER.c_str(), MQTT_PORT);
  Serial.println(MQTT_BROKER);
  Serial.println(deviceName);
  Serial.println(mqtt_user);
  Serial.println(mqtt_password);
  while (! mqttclient.connect((char*)deviceName.c_str(), (char*)mqtt_user.c_str(), (char*)mqtt_password.c_str()) && count_round < 100)
  {
    Serial.println("retry mqtt connect");
    delay(500);
    count_round++;
  }
}

void send_mqtt() {
  int a = 0;
  String topic;
  Serial.println("enter send_mqtt");
  for ( int i = 0 ; i < 6 ; i++)
  {
    Serial.println(i);
    switch ( i )
    {
      case 0:
        //0->Voltage
        dtostrf(volt, 8, 2, buf);
        break;
      case 1:
        //1->Temp
        dtostrf(bme280.readTempC(), 8, 2, buf);
        break;
      case 2:
        //2->Humidity
        dtostrf(bme280.readHumidity(), 8, 2, buf);
        break;
      case 3:
        //3->Pressure
        dtostrf(bme280.readPressure(), 8, 2, buf);
        break;
      case 4:
        //4->AltitudeMeter
        dtostrf(bme280.readAltitudeMeter(), 8, 2, buf);
        break;
      case 5:
        //5->DewPoint
        dtostrf((calc_dewpoint(int(bme280.readHumidity()) , int(bme280.readTempC()))), 8, 2, buf);
        break;
      default:
        buf[0] = '\n';
    }
    topic=TOPIC_ROOT+topics[i];
    mqttclient.publish(topic.c_str(), buf);
  }
  Serial.println("leave send_mqtt");
}

void flip()
{
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
  if ( blink_count <= 0 )
  {
    flipper.detach();
    digitalWrite(LED_BUILTIN, HIGH);
  }
  blink_count--;
}

void blink_now(float interval, int count )
{
  blink_count = count;
  flipper.attach(interval, flip);
  while (blink_count > 0)
  {
    delay(5000);
    Serial.println(blink_count);
  }
}


void setup() {
  int count_round = 0;
  Serial.begin(74880); // Aufbau einer seriellen Verbindung
  Serial.setTimeout(2000);
  //read the config values
  //const char* wifi_ssid = "MYWLAN_GAST";
  //const char* wifi_password = "WLANGAST";
//const char* deviceName = "accudurationtest1";
//const char* mqtt_user = "weather1";
//const char* mqtt_password = "transportdata";
//const char* MQTT_BROKER = "192.168.2.101";
//const int MQTT_PORT = 1883;
  wifi_ssid=CONFIG.getConfigValue("wifi_ssid");
  wifi_password=CONFIG.getConfigValue("wifi_password");
  deviceName=CONFIG.getConfigValue("deviceName");
  mqtt_user=CONFIG.getConfigValue("mqtt_user");
  mqtt_password=CONFIG.getConfigValue("mqtt_password");
  MQTT_BROKER=CONFIG.getConfigValue("MQTT_BROKER");
  MQTT_PORT=CONFIG.getConfigValue("MQTT_PORT").toInt();
  TOPIC_ROOT=CONFIG.getConfigValue("TOPIC_ROOT");

  /* just for testing without hardware
  connect_wifi();
  connect_mqtt();
  send_mqtt();
  */
  
  while (!Serial) {};
  //init led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  //useless pinMode(D0, WAKEUP_PULLUP);
  //init port for read
  pinMode(A0, INPUT);
  //0 is I2C
  bme280.parameter.communication = 0;
  bme280.parameter.I2CAddress = 0x76;                  //Choose I2C Address
  //0b00:     In sleep mode no measurements are performed, but power consumption is at a minimum
  //0b01:     In forced mode a single measured is performed and the device returns automatically to sleep mode
  //0b11:     In normal mode the sensor measures continually (default value)
  bme280.parameter.sensorMode = 0b11;                   //Choose sensor mode
  //0b000:      factor 0 (filter off)
  //0b001:      factor 2
  //0b010:      factor 4
  //0b011:      factor 8
  //0b100:      factor 16 (default value)
  bme280.parameter.IIRfilter = 0b100;                    //Setup for IIR Filter
  bme280.parameter.humidOversampling = 0b101;            //Setup Humidity Oversampling
  bme280.parameter.tempOversampling = 0b101;             //Setup Temperature Ovesampling
  bme280.parameter.pressOversampling = 0b101;            //Setup Pressure Oversampling
  //bme280.parameter.pressureSeaLevel = 1013.25;           //default value of 1013.25 hPa
  bme280.parameter.pressureSeaLevel = 1011.42;           //50 meter über see level https://www.sensorsone.com/altitude-pressure-units-conversion/
  bme280.parameter.tempOutsideCelsius = 9.3;              //lwikipedia default value of 15°C
  if (bme280.init() != 0x60)
  {
    Serial.println(F("Ops! BME280 could not be found!"));
  }
  else
  {
    Serial.println(F("BME280 detected!"));
  }
  // Wait for serial to initialize.
  while (!Serial) {};
  state.registerVar( &volt );
  state.registerVar( &channel );
  state.registerVar( bssid );
  if (state.loadFromRTC()) {            // we load the values from rtc memory back into the registered variables
    Serial.println("push reset or back from deep sleep");
    //state.saveToRTC();                  // since we changed a state relevant variable, we store the new values
  }
  else
  {
    Serial.println("This seems to be a cold boot. We don't have a valid state on RTC memory");
  }
  state.saveToRTC();

}

void loop() {
  switch ( get_powerstate() ) {
    case 0:
      Serial.println("powerstate 0");
      //state.debugOutputRTCVars();
      connect_wifi();
      connect_mqtt();
      send_mqtt();
      //blink_now(0.7, 10);
      //wifi + mqtt!
      mqttclient.disconnect();
      delay(500);
      WiFi.disconnect();
      WiFi.forceSleepBegin();
      delay(1);
      Serial.flush();
      Serial.end();
      //blink_now(0.2,20);
      //10 min
      ESP.deepSleep(300e6);
      //ESP.deepSleep(30e6);
      break;
    case 1:
      Serial.println("powerstate 1 deep sleep deepSleepMax/2");
      connect_wifi();
      connect_mqtt();
      send_mqtt();
      //blink_now(0.5, 10);
      mqttclient.disconnect();
      delay(500);
      WiFi.disconnect();
      WiFi.forceSleepBegin();
      delay(1);
      Serial.flush();
      Serial.end();
      //blink_now(0.2,20);
      ESP.deepSleep((ESP.deepSleepMax() / 2));
      break;
    case 2:
      Serial.println("powerstate 2 deep sleep deepSleepMax!");
      mqttclient.disconnect();
      delay(500);
      WiFi.disconnect();
      WiFi.forceSleepBegin();
      //Serial.println("should be a deep sleep implementation");
      // Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
      //connect GPIO 16 (D0) to RST 330 – 1kΩ between D0
      Serial.println("deep sleep max");
      //ESP.deepSleep(ESP.deepSleepMax());
      Serial.flush();
      Serial.end();
      //blink_now(0.2,20);
      ESP.deepSleep(ESP.deepSleepMax());
      break;
  }
}
