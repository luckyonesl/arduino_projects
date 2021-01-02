
#include <Arduino.h>
#include "ConfigReader.h"
#include <ESP8266WiFi.h>
//https://github.com/tsi-software/Secure_ESP8266_MQTT_poc/blob/master/top-level-components/secure_esp8266_mqtt_client/SetupWifi.h
#include <PubSubClient.h>
#include <RTCVars.h>
#include <Ticker.h>
#include "BlueDot_BME280.h"
#include "utilities.h"

ConfigReader CONFIG("/config.txt");

BlueDot_BME280 bme280 = BlueDot_BME280();
RTCVars state; // create the state object

char buf[50];


String wifi_ssid;
String wifi_password;
String deviceName;
String mqtt_user;
String mqtt_password;
String MQTT_BROKER;
String TOPIC_ROOT;
int MQTT_PORT;
bool timer_is_attached=false;
unsigned long start_wifi;

int channel;  // 1 byte,   5 in total
byte bssid[6]; // 6 bytes, 11 in total
int powerstate;
float volt;

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
Ticker no_wifi_ticker_signal;
long blink_count;

void led_signal()
{
  //Serial.println("led_signal");
  //led signals send
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
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
    topic = TOPIC_ROOT + topics[i];
    mqttclient.publish(topic.c_str(), buf);
  }
  Serial.println("leave send_mqtt");
}

void setup() {
  int count_round = 0;
  Serial.begin(74880); // Aufbau einer seriellen Verbindung
  Serial.setTimeout(2000);
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
  start_wifi = millis();
  while (!Serial) {};
  //wifi should only be called if not powerstate 2
  powerstate=get_powerstate();
  Serial.print("powerstate is ");
  Serial.println(powerstate);
  if ( powerstate < 2 )
  {
    Serial.println("establish wifi");
    wifi_establish_connection(wifi_ssid, wifi_password, deviceName);
  }
  //init led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  volt=getBatteryVoltage();
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
  state.registerVar( &powerstate );
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
  Serial.println("leave setup");
}

void loop() {
  long bme280wait;
  int pwst=get_powerstate();

  //Serial.println("enter loop");
  if ( ! wifi_is_connected() )
  {
    if ( ( start_wifi + 100000 ) < millis() )
    {
      Serial.println("got no wifi connection reset now");
      wifi_teardown_rst();
    }
    if ( ! timer_is_attached )
    {
      no_wifi_ticker_signal.attach(1,led_signal);  
      timer_is_attached=true;
    }
    //in case of powerstate 2 deepsleep
    //Serial.print ("powerstate is ");
    //Serial.println(pwst);
    if ( pwst == 2 ) { ESP.deepSleep(ESP.deepSleepMax()); }
    Serial.print(".");
    delay(300);
    return;
  }
  else
  {
    //deatach led signal
    if ( timer_is_attached == true )
    {
      no_wifi_ticker_signal.detach(); 
      timer_is_attached=false;
      delay(300);
      //switch of blinking
      digitalWrite(LED_BUILTIN, HIGH);  
    }  
  }
  bme280wait=(start_wifi + 20000 ) - millis();
  if ( bme280wait > 0 )
  {
    //for bme280 to get reasonable values
    Serial.print("wait for ");Serial.println(bme280wait);
    delay(bme280wait);
  }
  switch ( powerstate ) {
    case 0:
      Serial.println("powerstate 0");
      //state.debugOutputRTCVars();
      if ( wifi_is_connected() ) 
      {
        connect_mqtt();
        send_mqtt();
      }
      mqttclient.disconnect();
      wifi_teardown();
      delay(500);
      Serial.flush();
      Serial.end();
      ESP.deepSleep(300e6);
      //ESP.deepSleep(30e6);
      break;
    case 1:
      Serial.println("powerstate 1 deep sleep deepSleepMax/2");
      if ( wifi_is_connected() ) 
      {
        connect_mqtt();
        send_mqtt();
      }
      //blink_now(0.5, 10);
      mqttclient.disconnect();
      wifi_teardown();
      delay(500);
      Serial.flush();
      Serial.end();
      ESP.deepSleep((ESP.deepSleepMax() / 2));
      break;
    case 2:
      Serial.println("powerstate 2 deep sleep deepSleepMax!");
      mqttclient.disconnect();
      wifi_teardown();
      delay(500);
      //Serial.println("should be a deep sleep implementation");
      // Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
      //connect GPIO 16 (D0) to RST 330 – 1kΩ between D0 in case of wemos mini with Schottky-Diode
      Serial.println("deep sleep max");
      //ESP.deepSleep(ESP.deepSleepMax());
      Serial.flush();
      Serial.end();
      ESP.deepSleep(ESP.deepSleepMax());
      break;
  }
}
