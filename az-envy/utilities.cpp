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

bool wifi_establish_connection( String wifi_ssid,String wifi_password, String deviceName)
{
  bool retval = true;
  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEventHndl);  
  WiFi.hostname(deviceName);
  WiFi.begin(wifi_ssid, wifi_password);
  return retval;
}
