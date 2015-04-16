/*
  Config.h - Library for configuration
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef Config_h
#define Config_h

#include "Arduino.h"
#include <EEPROM.h>
#include <RFM69.h>

class Config {
  public:
    Config(bool init);
    void loadConfig();
    void storeConfig();
    byte rfNodeId;
    byte rfNetworkId;
    byte rfGatewayId;
    byte rfFrequency;
    byte loopMultiplier;
    byte alertMultiplier;
  protected:
    void initConfig();
};

#endif
