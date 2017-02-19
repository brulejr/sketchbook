/* -----------------------------------------------------------------------------
   ESP8266 Water Sensor
  
   Monitors water overflow as well as temperature and humidity watching for out-of-ordinary behavior.
 
   Circuit:
   * ESP8266
   * Door sensor attached to digitial pin 2
   * Temperature/Humidity (DHT11) sensor attached to digital pin 3
   * Water sensor attached to digital pin 8
 
   Created 22-JAN-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "MQTT.h"

#define DEVICE_NAME     "water-mote-01"
#define RESET_SETTINGS  false
MQTT mqtt(DEVICE_NAME, RESET_SETTINGS);

#define DOOR_PIN 2
int doorState;

boolean startup = true;

void setup() {
  Serial.begin(115200);
  mqtt.setup();

  pinMode(DOOR_PIN, INPUT_PULLUP);
  doorState = digitalRead(DOOR_PIN);
  attachInterrupt(digitalPinToInterrupt(DOOR_PIN), _doorStateChange, CHANGE);

  startup = false;
}

void loop() {
  mqtt.check();
  delay(10000);
}

void _doorStateChange() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (!startup && (interrupt_time - last_interrupt_time > 200)) {
    doorState = digitalRead(DOOR_PIN);
    String json = "{\"door\": \"";
    json += (doorState == LOW ? "OPEN" : "CLOSED");
    json += "\"}";
    Serial.print("Alert: "); Serial.println(json);
    mqtt.publish("alert", json.c_str());
  }
  last_interrupt_time = interrupt_time;
}
