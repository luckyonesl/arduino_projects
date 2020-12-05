//#include "SPIFFS.h"
//for esp8266
#include <FS.h>
//ide tool for uploading
//Sketchbook location tools and extract inside this dir
//https://github.com/esp8266/arduino-esp8266fs-plugin/releases
//in the ide ESP8266 Sketch Data Upload

void setup() {
  Serial.begin(74880); // Aufbau einer seriellen Verbindung
  Serial.setTimeout(2000);
  //if ( SPIFFS.format() ) { Serial.println("format"); };
  while (!Serial) {};
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    bool formatted = SPIFFS.format();
    if ( formatted ) {
      Serial.println("SPIFFS formatted successfully");
      SPIFFS.begin();
    } else {
      Serial.println("Error formatting");
    }
  }
  if ( SPIFFS.exists("/test.txt") ) {
    Serial.println("file test.txt already exist");
  }

  //write to file
  File file = SPIFFS.open("/test.txt", "a");
  if (!file) {
    // File not found
    Serial.println("Failed to open test file");
    return;
  } else {
    int bytesWritten = file.println("Hello From ESP32");
    file.close();
    Serial.println("File was written");
    Serial.println(bytesWritten);
  }
}

// the loop function runs over and over again forever
void loop() {
  /*unsigned int totalBytes = SPIFFS.totalBytes();
    unsigned int usedBytes = SPIFFS.usedBytes();
    Serial.println("===== File system info =====");

    Serial.print("Total space:      ");
    Serial.print(totalBytes);
    Serial.println("byte");

    Serial.print("Total space used: ");
    Serial.print(usedBytes);
    Serial.println("byte");
  */
  Serial.println("===== in loop =====");
  File file2 = SPIFFS.open("/test.txt", "r");
  if (!file2) {
    // File not found
    Serial.println("Failed to open test file");
    return;
  } else {
    //print content
    Serial.println("open test file");
    while (file2.available()) {
      Serial.print("in while");
      String line = file2.readStringUntil('\n');
      Serial.println(line);
      //  Serial.write(file2.read());
    }
    file2.close();
  }

  delay(10000);

}
