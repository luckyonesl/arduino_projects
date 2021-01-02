#ifndef utilities_h
#define utilities_h
//WiFi.printDiag(Serial);
void WiFiEventHndl(WiFiEvent_t event);
bool wifi_is_connected();
bool wifi_teardown();
void wifi_teardown_rst(int max,int* curval);
bool wifi_establish_connection( String wifi_ssid,String wifi_password,String deviceName);
float calc_dewpoint(unsigned int h , int t);
int get_powerstate();
float getBatteryVoltage();
#endif
