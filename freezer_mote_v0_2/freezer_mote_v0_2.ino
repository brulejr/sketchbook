/* -----------------------------------------------------------------------------
   Freezer Sensor
  
   Monitors several sensors on a freezer watching for out-of-ordinary behavior.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * Battery voltage monitor attached to analog pin 0
   * LDR sensor attached to analog pin 1
   * Hall Effect sensor attached to digital pin 7
   * RF Status LED attached to digital pin 9
 
   Created 08-JUN-2014 by Jon Brule
----------------------------------------------------------------------------- */
#include <ChibiOS_AVR.h>
#include <Heartbeat.h>
#include "Freezer.h"

#define VERSION    "v0.2"

#define SERIAL      1   // set to 1 to also report readings on the serial port
#define DEBUG       1   // set to 1 to display each loop() run
#define BAUD_RATE   57600

#define DPIN_MOTE_LED    9  // moteinos have LEDs on D9

#define MEASURE_PERIOD   1000
#define REPORT_INTERVAL  10

//----------------------------------------------------------------------------- 
// heartbeat thread
//
static WORKING_AREA(waThread1, 50);
static msg_t HeartbeatThread(void *arg) {
  Heartbeat heartbeat(DPIN_MOTE_LED);
  while (1) {
    heartbeat.pulse();
  }
  return 0;
}

//----------------------------------------------------------------------------- 
// freezer thread
//
static WORKING_AREA(waThread2, 50);
static msg_t FreezerThread(void *arg) {
  Freezer freezer(REPORT_INTERVAL);
  while (1) {
    freezer.measure();
    chThdSleepMilliseconds(MEASURE_PERIOD);
  }
  return 0;
}

//-----------------------------------------------------------------------------
void setup () {
  
    #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[freezerMote - ");
        Serial.print(VERSION);
        Serial.println("]");
    #endif
    
    chBegin(mainThread);
}

//-----------------------------------------------------------------------------
void mainThread () {
  
  // start heartbeat thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 1, HeartbeatThread, NULL);
  
  // start freezer thread
  chThdCreateStatic(waThread2, sizeof(waThread2),
                    NORMALPRIO + 2, FreezerThread, NULL);

  while (true)
    loop();
}

//-----------------------------------------------------------------------------
void loop () {
  delay(1000);
}
