/*
  MQTT.h - Encapsulates MQTT / PubSubClient operations
  Created by Jon R. Brule, January 22, 2017.
*/
#ifndef MQTT_h
#define MQTT_h

#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

class MQTT
{
  public:
    MQTT(byte* macAddr, char* mqttServer, char* deviceName);
    void check();
    void publish(char* topic, char* message);
    void setup();
  private:
    EthernetClient _ethClient;
    PubSubClient _pubSubClient;
    char* _deviceName;
    byte* _macAddr;
    char* _mqttServer;
};

#endif
