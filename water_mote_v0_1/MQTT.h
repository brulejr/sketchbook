/*
  MQTT.h - Encapsulates MQTT / PubSubClient operations
  Created 22-JAN-2017 by Jon Brule
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
    MQTT(char* deviceName, byte* macAddr, char* mqttServer, char* mqttUserid, char* mqttPasswd);
    void check();
    void publish(char* topic, char* message);
    void setup();
  private:
    EthernetClient _ethClient;
    PubSubClient _pubSubClient;
    char* _deviceName;
    byte* _macAddr;
    char* _mqttServer;
    char* _mqttUserId;
    char* _mqttPasswd;
};

#endif
