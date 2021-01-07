#include <ESP8266WiFi.h>
bool wifi_connected=false;

bool wifi_is_connected()
{
  if ( wifi_connected )
    return true;
  return wifi_connected;
}

void WiFiEventHndl(WiFiEvent_t event) //, WiFiEventInfo_t info
{
  switch (event) {
    case WIFI_EVENT_STAMODE_CONNECTED:
      //enable sta ipv6 here
      //WiFi.enableIpV6();
      break;
    case WIFI_EVENT_STAMODE_GOT_IP:
      //Serial.print("wifi connected");
      //wifiOnConnect();
      wifi_connected = true;
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      wifi_connected = false;
      break;
    case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
      Serial.println("WIFI_EVENT_STAMODE_AUTHMODE_CHANGE");
      break;
    case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
      Serial.println("WIFI_EVENT_STAMODE_DHCP_TIMEOUT");
      break;     
    default:
      break;
  }

}

bool wifi_teardown()
{
  bool retval=true;
  WiFi.disconnect();
  delay(1000); 
  WiFi.forceSleepBegin();
  delay(1000);   
  return retval;
}

void wifi_teardown_rst(int max,int* curval)
{
  //Serial.print("max is"); Serial.println(max);
  //Serial.print("curval is"); Serial.println(*curval);
  *curval = *curval + 1 ;
  if (*curval>max)
  {
    wifi_teardown();
    //end the wlan and reset device
    ESP.reset();
  }
  delay(1000);
}

void wifi_teardown_rst()
{
  wifi_teardown();
  delay(5000);
  ESP.reset();
}

bool wifi_establish_connection( String wifi_ssid,String wifi_password, String deviceName)
{
  bool retval = true;
  WiFi.persistent(false);
  // only if we want.. is part of tear down WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEventHndl);  
  WiFi.hostname(deviceName);
  WiFi.begin(wifi_ssid, wifi_password);
  return retval;
}


float calc_dewpoint(unsigned int h , int t)
{
  float H, dew_point;
  H = (log10(h) - 2.0) / 0.4343 + (17.62 * t) / (243.12 + t);
  dew_point = 243.12 * H / (17.62 - H);
  return dew_point;
}

float getBatteryVoltage()
{
  unsigned int raw = 0;
  float myvolt = 0.0;  
  //init port for read
  pinMode(A0, INPUT);
  raw = analogRead(A0);
  myvolt = raw / 1023.0;
  myvolt = myvolt * 4.2;
  //Serial.print("Voltage is ");
  //Serial.println(myvolt);
  return myvolt;
}

int get_powerstate() {
  int pwst;
  float volt=getBatteryVoltage();
  //0 no save 1 switch off wifi 2 sleep 3 dep stande by
  //pin is set
  if ( volt < 1 )
  {
    //Serial.println("no bat keep state 0");
    pwst = 0;    
  }
  if ( volt > 3.6 )
  {
    pwst = 0;
    //Serial.println("state 0, online");
  }
  if ( volt <= 3.6 && volt > 3.4 )
  {
    pwst = 1;
    //Serial.println("state 1, online but switch of wifi");
  }
  if ( volt <= 3.4 && volt > 1)
  {
    pwst = 2;
    //Serial.println("state 2, deep sleep max");
  }
  //3 is missing  
  return pwst;
}
