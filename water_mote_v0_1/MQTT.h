/*
  MQTT.h - Encapsulates MQTT / PubSubClient operations
  Created by Jon R. Brule, January 22, 2017.
*/
#ifndef MQTT_h
#define MQTT_h

#include "Arduino.h"
#include <Ethernet.h>
#include <PubSubClient.h>

class MQTT
{
  public:
    MQTT(EthernetClient* ethClient, char* server);
    void check();
    void publish(char* topic, char* message);
  private:
    PubSubClient _pubSubClient;  
};

#endif
