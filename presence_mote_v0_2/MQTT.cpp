/*
  MQTT.cpp - Encapsulates MQTT / PubSubClient operations
  Created 26-JAN-2017 by Jon Brule
*/
#include "Arduino.h"
#include "MQTT.h"

//------------------------------------------------------------------------------
// checks connectivity
//
MQTT::MQTT(bool resetSettings) {
  _resetSettings = resetSettings;
  _espClient = WiFiClient();
  _pubSubClient = PubSubClient(_espClient);  
}

//------------------------------------------------------------------------------
// checks connectivity
//
void MQTT::check() {
  if (!_pubSubClient.connected()) {
    while (!_pubSubClient.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (_pubSubClient.connect(_deviceName, _mqttUserId, _mqttPasswd)) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        publish("status","CONNECTED");
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
void MQTT::publish(char* topic, const char* message) {
  String topicPath = _deviceName;
  topicPath += "/";
  topicPath += topic;
  _pubSubClient.publish(topicPath.c_str(), message);
}

//------------------------------------------------------------------------------
// setup 
//
void MQTT::setup() {
  _mountFFS();
  _setupWifi();
  _writeConfig();
  Serial.print("local ip - "); Serial.println(WiFi.localIP());
  _setupMQTT();
}

//------------------------------------------------------------------------------
// mount file system
//
void MQTT::_mountFFS() {
  if (_resetSettings) {
    Serial.println("formatting FS...");
    SPIFFS.format();
  }
  
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(_mqttServer, json["mqtt_server"]);
          strcpy(_mqttPort, json["mqtt_port"]);
          strcpy(_mqttUserId, json["mqtt_userid"]);
          strcpy(_mqttPasswd, json["mqtt_passwd"]);
          strcpy(_deviceName, json["device_name"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }  
}

//------------------------------------------------------------------------------
// configure mqtt settings
//
void MQTT::_setupMQTT() {
  _pubSubClient.setServer(_mqttServer, atoi(_mqttPort));
}

//------------------------------------------------------------------------------
// configure wifi settings
//
void MQTT::_setupWifi() {
  WiFiManager wifiManager;
  
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", _mqttServer, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", _mqttPort, 6);  
  WiFiManagerParameter custom_mqtt_userid("userid", "mqtt userid", _mqttUserId, 16);  
  WiFiManagerParameter custom_mqtt_passwd("passwd", "mqtt passwd", _mqttPasswd, 16);  
  WiFiManagerParameter custom_device_name("deviceName", "device name", _deviceName, 32);  
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_userid);
  wifiManager.addParameter(&custom_mqtt_passwd);
  wifiManager.addParameter(&custom_device_name);
  
  if (_resetSettings) {
    wifiManager.resetSettings();
  }
  
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }
  
  strcpy(_mqttServer, custom_mqtt_server.getValue());
  strcpy(_mqttPort, custom_mqtt_port.getValue());    
  strcpy(_mqttUserId, custom_mqtt_userid.getValue());    
  strcpy(_mqttPasswd, custom_mqtt_passwd.getValue());    
  strcpy(_deviceName, custom_device_name.getValue());    
}

//------------------------------------------------------------------------------
// save configuration
//
void MQTT::_writeConfig() {
  Serial.print("saving config - ");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["mqtt_server"] = _mqttServer;
  json["mqtt_port"] = _mqttPort;
  json["mqtt_userid"] = _mqttUserId;
  json["mqtt_passwd"] = _mqttPasswd;
  json["device_name"] = _deviceName;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
  Serial.println();
}

