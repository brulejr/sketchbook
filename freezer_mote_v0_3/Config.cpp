/*
  Config.cpp - Logic for configuration
  Created by Jon R. Brule, April 14, 2015.
  Released into the public domain.
*/
#include "Config.h"

#define DEBUG             1

#define NODEID            9   // unique for each node on same network
#define GATEWAYID         1
#define NETWORKID         99  // same for all nodes that talk to each other
#define FREQUENCY         RF69_915MHZ

#define LOOP_MULTIPLIER   10
#define ALTER_MULTIPLIER  2


//-----------------------------------------------------------------------------
Config::Config(bool init) {
  if (init) {
    initConfig();
    storeConfig();
  } 
  loadConfig();
}

//-----------------------------------------------------------------------------
void Config::initConfig() {
  rfNodeId = NODEID;
  rfNetworkId = NETWORKID;
  rfGatewayId = GATEWAYID;
  rfFrequency = FREQUENCY;
  loopMultiplier = LOOP_MULTIPLIER;
  alertMultiplier = ALTER_MULTIPLIER;
}

//-----------------------------------------------------------------------------
void Config::loadConfig() {
  #if DEBUG
    Serial.print("loading configuration from eeprom...");
  #endif
  rfNodeId = EEPROM.read(0);
  rfNetworkId = EEPROM.read(1);
  rfGatewayId = EEPROM.read(2);
  rfFrequency = EEPROM.read(3);
  loopMultiplier = EEPROM.read(4);
  alertMultiplier = EEPROM.read(5);
  #if DEBUG
    Serial.println("ok!");
  #endif
}

//-----------------------------------------------------------------------------
void Config::storeConfig() {
  #if DEBUG
    Serial.print("storing configuration to eeprom...");
  #endif
  EEPROM.write(0, rfNodeId);
  EEPROM.write(1, rfNetworkId);
  EEPROM.write(2, rfGatewayId);
  EEPROM.write(3, rfFrequency);
  EEPROM.write(4, loopMultiplier);
  EEPROM.write(5, alertMultiplier);
  #if DEBUG
    Serial.println("ok!");
  #endif
}

