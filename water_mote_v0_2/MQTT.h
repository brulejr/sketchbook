/*
  MQTT.h - Encapsulates MQTT / PubSubClient operations
  Created 26-JAN-2017 by Jon Brule
*/
#ifndef MQTT_h
#define MQTT_h

#include "Arduino.h"

#include <FS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

class MQTT
{
  public:
    MQTT(char* deviceName, bool resetSettings);
    void check();
    void publish(char* topic, char* message);
    void setup();
  private:
    WiFiClient _espClient;
    PubSubClient _pubSubClient;
    char* _deviceName;
    bool _resetSettings;
    char _mqttServer[40];
    char _mqttPort[6];
    char _mqttUserId[16];
    char _mqttPasswd[16];
    void _mountFFS();
    void _setupMQTT();
    void _setupWifi();
    void _writeConfig();
};

#endif
