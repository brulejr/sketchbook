/* -----------------------------------------------------------------------------
   Freezer Mote Sensor
  
   Monitors a freezer.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * A0  - Battery level detector
   * A1  - Light detector
   * A2  - Temperature (inside)
   * A3  - Temperature (outside)
   * D3  - Door detector
   * D7  - External heartbeat LED
   * D9  - Internal heartbeat LED
 
   Created 13-APR-2015 by Jon Brule
----------------------------------------------------------------------------- */

#include "FreezerMote.h"
#include <EEPROM.h>
#include <LowPower.h>
#include <Message.h>
#include <RFM69.h>
#include <SPI.h>

#define MOTE       "sample-mote"
#define VERSION    "v0.1"

#define BAUD_RATE  9600

FreezerMoteConfig config;
FreezerMote* mote;

void setup() {
  Serial.begin(BAUD_RATE);
  
  config.rfNodeId = 9;
  
  mote = new FreezerMote(MOTE, VERSION, &config, true);
  mote->setup();
}

void loop() {
  mote->loop();
}
