#include <Wire.h> //include Wire.h library

void setup()
{
  Wire.begin(); // Wire communication begin
  Serial.begin(74880);
  while (!Serial); // Waiting for Serial Monitor
  Serial.println("leave setup");
}

void loop()
{
  byte err;
  byte addr; 
  int devcnt=0;

  Serial.println("enter loop for i2c scan...");
  for (addr = 1; addr < 127; addr++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();

    if (err == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (addr < 16)
      {
        Serial.print("0");
      }
      Serial.print(addr, HEX);
      Serial.println("  !");
      devcnt++;
    }
    else if (err == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (addr < 16)
      {
        Serial.print("0");
      }
      Serial.println(addr, HEX);
    }
  }
  if (devcnt == 0)
  {
    Serial.println("No devices found\n");
  }
  else
  {
    Serial.print("found ");
    Serial.print(devcnt);
    Serial.print(" devie(s)");
    Serial.println("done\n");
  }

  delay(50000); // wait 5 seconds for the next I2C scan
}
