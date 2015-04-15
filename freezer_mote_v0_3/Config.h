/*
  Config.h - Library for configuration
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include <EEPROM.h>

typedef struct ConfigData {
  byte nodeId;
  byte networkId;
  byte gatewayId;
};

class Config {
  public:
    Config(bool init);
    void load();
    void store();
    ConfigData data; 
};

#endif
