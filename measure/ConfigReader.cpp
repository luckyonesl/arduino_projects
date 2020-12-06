#include "Arduino.h"
#include "ConfigReader.h"

void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    // abort program execution.
    abort();
}

ConfigReader::ConfigReader(const String configfile)
{
  //maybe check if it's existing
  ConfigFile = configfile;
}

String ConfigReader::getConfigValue(char *keyname)
{
  String VALUE;
  int countline=0;
  if (!SPIFFS.begin()) {
    assert(false);  
    return VALUE;
  }
  if ( SPIFFS.exists(ConfigFile) ){
    //read line by line and search for keyname and return after split = value
    File file = SPIFFS.open(ConfigFile, "r");
    while (file.available()) {
      countline++;
      String line = file.readStringUntil('\n');
      if ( line.startsWith(keyname) )
      {
        int pos=line.indexOf('=');
        if ( pos > 0 ){
          pos++;
          VALUE=line.substring(pos);   
        }
        else {
          VALUE='\n';  
        }        
      }
    }
    file.close();
  }
  return VALUE;
}
