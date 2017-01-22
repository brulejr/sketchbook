/*
  MQTT.cpp - Encapsulates MQTT / PubSubClient operations
  Created by Jon R. Brule, January 22, 2017.
*/
#include "Arduino.h"
#include "MQTT.h"

MQTT::MQTT(byte* macAddr, char* server) {
  _macAddr = macAddr;
  _server = server;
  _ethClient = EthernetClient();
  _pubSubClient = PubSubClient(_ethClient);
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

//------------------------------------------------------------------------------
// checks connectivity
//
void MQTT::setup() {
  // setup ethernet
  delay(10);
  Serial.print("Attempting to connect to network...");
  if (Ethernet.begin(_macAddr) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }  
  delay(1000);
  Serial.println(Ethernet.localIP());

  // setup MQTT
  _pubSubClient.setServer(_server, 1883);
}
