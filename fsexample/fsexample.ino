#include "SPIFFS.h" 

void setup() {
  Serial.begin(74880); // Aufbau einer seriellen Verbindung
  Serial.setTimeout(2000);
  while (!Serial) {};
  if(!SPIFFS.begin()){ 
    Serial.println("An Error has occurred while mounting SPIFFS");  
    bool formatted = SPIFFS.format();
    if ( formatted ) {
      Serial.println("SPIFFS formatted successfully");
      SPIFFS.begin();
    }else {
      Serial.println("Error formatting");
    }
  }
   if ( SPIFFS.exists("/test.txt") ){
      Serial.println("file test.txt already exist");   
   }
  
  //write to file
   file = SPIFFS.open("/test.txt", "a");
   if(!file){
      // File not found
      Serial.println("Failed to open test file");
      return;
   } else {
      file.println("Hello From ESP32 :-)");
      file.close();
   }
}

// the loop function runs over and over again forever
void loop() {
   unsigned int totalBytes = SPIFFS.totalBytes();
   unsigned int usedBytes = SPIFFS.usedBytes();
   Serial.println("===== File system info =====");
 
   Serial.print("Total space:      ");
   Serial.print(totalBytes);
   Serial.println("byte");
 
   Serial.print("Total space used: ");
   Serial.print(usedBytes);
   Serial.println("byte");

   delay(1000);
}
