/*
  Config.cpp - Logic for configuration
  Created by Jon R. Brule, April 14, 2015.
  Released into the public domain.
*/
#include "Config.h"

#define DEBUG            1

#define NODEID           9   // unique for each node on same network
#define GATEWAYID        1
#define NETWORKID        99  // same for all nodes that talk to each other
#define FREQUENCY        RF69_915MHZ


//-----------------------------------------------------------------------------
Config::Config(bool init) {
  if (init) {
    store();
  } 
  load();
}

//-----------------------------------------------------------------------------
void Config::load() {
  #if DEBUG
    Serial.print("loading configuration from eeprom...");
  #endif
  data.nodeId = EEPROM.read(0);
  data.networkId = EEPROM.read(1);
  data.gatewayId = EEPROM.read(2);
  #if DEBUG
    Serial.println("ok!");
  #endif
}

//-----------------------------------------------------------------------------
void Config::store() {
  #if DEBUG
    Serial.print("loading configuration from eeprom...");
  #endif
  EEPROM.write(0, NODEID);
  EEPROM.write(1, NETWORKID);
  EEPROM.write(2, GATEWAYID);
  #if DEBUG
    Serial.println("ok!");
  #endif
}

