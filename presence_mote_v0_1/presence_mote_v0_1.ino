/* -----------------------------------------------------------------------------
   ESP8266 Presence Sensor
  
   Monitors occupancy of a room.
 
   Circuit:
   * ESP8266
   * Door sensor attached to digitial pin 2
 
   Created 28-JAN-2017 by Jon Brule
----------------------------------------------------------------------------- */

#include "MQTT.h"

#define RESET_SETTINGS  false
MQTT mqtt(RESET_SETTINGS);

#define DOOR_PIN 2
volatile int doorState;
volatile boolean stateChange = false;

#define PIR_PIN 12
int pirState;

#define LDR_PIN A0
int ldrState;

boolean startup = true;

void setup() {
  Serial.begin(115200);
  mqtt.setup();

  pinMode(DOOR_PIN, INPUT_PULLUP);
  doorState = digitalRead(DOOR_PIN);
  attachInterrupt(digitalPinToInterrupt(DOOR_PIN), _doorStateChange, CHANGE);

  pinMode(PIR_PIN, INPUT);
  pirState = digitalRead(PIR_PIN);

  ldrState = analogRead(A0);

  startup = false;
}

void loop() {
  mqtt.check();
  if (stateChange) {
    String json = "{\"door\": \"";
    json += (doorState == LOW ? "OPEN" : "CLOSED");
    json += "\"}";
    Serial.print("Alert: "); Serial.println(json);
    mqtt.publish("alert", json.c_str());    
    stateChange = false;
  }
  if (doorState == HIGH) {
    pirState = digitalRead(PIR_PIN);
    if (pirState == HIGH) {
      String json = "{\"door\": \"CLOSED\", \"motion\": \"DETECTED\"}";
      Serial.print("Alert: "); Serial.println(json);
      mqtt.publish("alert", json.c_str());
    }
  }

  ldrState = analogRead(A0);
  Serial.print("LDR = "); Serial.println(ldrState);
  delay(1000);
}

void _doorStateChange() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (!startup && (interrupt_time - last_interrupt_time > 200)) {
    doorState = digitalRead(DOOR_PIN);
    stateChange = true;
  }
  last_interrupt_time = interrupt_time;
}
