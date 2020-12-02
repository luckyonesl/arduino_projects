/*
Wemos 	   Gas Sensor
3v3 	      VCC
GND 	      GND
NC 	      DO
Analog A0 	AO
*/
//should be used like here
//https://www.instructables.com/Arduino-CO-Monitor-Using-MQ-7-Sensor/
//https://github.com/suryasundarraj/mq7-esp8266-thingspeak/
//https://iot-playground.com/blog/2-uncategorised/53-esp8266-wifi-gas-sensor-arduino-ide
// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(74880); // Aufbau einer seriellen Verbindung
  Serial.setTimeout(2000);
  while (!Serial) {};
}

// the loop function runs over and over again forever
void loop() {
  float sensorVoltage; 
  float sensorValue;
 
  sensorValue = analogRead(A0);
  sensorVoltage = sensorValue/1024*5.0;
 
  Serial.print("sensor voltage = ");
  Serial.print(sensorVoltage);
  Serial.println(" V");
  delay(1000);
}
