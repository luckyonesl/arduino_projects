#include <Ticker.h>
Ticker flipper;
long blink_count;

void flip()
{
  int state = digitalRead(LED_BUILTIN);  
  digitalWrite(LED_BUILTIN, !state);    
  if ( blink_count <= 0 )
  {
    flipper.detach();
    digitalWrite(LED_BUILTIN, HIGH);
  }
  blink_count--;
}

void blink_now(float interval, int count )
{
  //we use the ticker no blocking
  blink_count = count;
  flipper.attach(interval, flip);
  while (blink_count > 0)
  {
    delay(5000);
    Serial.println(blink_count);
  }
}



// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  //if i enter i set it to low
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(74880); // Aufbau einer seriellen Verbindung
  Serial.setTimeout(2000);
  while (!Serial) {};
  blink_now(0.1, 10);
}

// the loop function runs over and over again forever
void loop() {
   //
   while (blink_count > 0 )
   {
      Serial.println("still blinking");
      delay(500);
   }
   ESP.deepSleep(4e6);
}
