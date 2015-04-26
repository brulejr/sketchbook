/*
  MoteConfig.h - Remote configuration
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef MoteConfig_h
#define MoteConfig_h

#include <RFM69.h>

typedef struct MoteConfig {
  byte rfNodeId = 2;
  byte rfNetworkId= 99;
  byte rfGatewayId = 1;
  byte rfFrequency = RF69_915MHZ;
  
  byte intStatusLedPin = 9;
  byte extStatusLedPin = 7;
  
  byte alertMultiplier = 2;
  byte loopMultiplier = 10;
};
#endif
