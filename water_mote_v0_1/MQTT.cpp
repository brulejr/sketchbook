/*
  MQTT.cpp - Encapsulates MQTT / PubSubClient operations
  Created by Jon R. Brule, January 22, 2017.
*/
#include "Arduino.h"
#include "MQTT.h"

MQTT::MQTT(EthernetClient* ethClient, char* server) {
    _pubSubClient = PubSubClient(*ethClient);
    _pubSubClient.setServer(server, 1883);
}

//------------------------------------------------------------------------------
// checks connectivity
//
void MQTT::check() {
  if (!_pubSubClient.connected()) {
    while (!_pubSubClient.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (_pubSubClient.connect("arduinoClient")) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        publish("outTopic","hello world!!");
//        _pubSubClient.publish("outTopic","hello world");
      } else {
        Serial.print("failed, rc=");
        Serial.print(_pubSubClient.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
  _pubSubClient.loop();
}

//------------------------------------------------------------------------------
// checks connectivity
//
void MQTT::publish(char* topic, char* message) {
  _pubSubClient.publish(topic, message);
}

