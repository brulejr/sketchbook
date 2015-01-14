/* -----------------------------------------------------------------------------
   Temperature Sensor
  
   Monitors several sensors on a temperature watching for out-of-ordinary behavior.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * Battery voltage monitor attached to analog pin 0
   * Temperature sensor attached to digital pin 3
   * RF Status LED attached to digital pin 9
 
   Created 08-JUN-2014 by Jon Brule
----------------------------------------------------------------------------- */
#include <ChibiOS_AVR.h>
#include <Heartbeat.h>
#include <RFM69.h>
#include <SPI.h>
#include <Message.h>
#include "Temperature.h"

#define VERSION    "v0.2"

#define SERIAL      1   // set to 1 to also report readings on the serial port
#define DEBUG       1   // set to 1 to display each loop() run
#define BAUD_RATE   57600

#define APIN_BATTERY     0
#define APIN_TEMPERATURE 1
#define DPIN_MOTE_LED    9  // moteinos have LEDs on D9

#define MEASURE_PERIOD   1000
#define REPORT_INTERVAL  5

#define NODEID        3   // unique for each node on same network
#define GATEWAYID     1
#define NETWORKID     99  // same for all nodes that talk to each other
#define FREQUENCY     RF69_915MHZ

Temperature temperature(REPORT_INTERVAL, APIN_TEMPERATURE, APIN_BATTERY);
RFM69 radio;

Message inbound, outbound;


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
// temperature thread
//
static WORKING_AREA(waThread2, 50);
static msg_t TemperatureThread(void *arg) {
  while (1) {
    temperature.measure();
    chThdSleepMilliseconds(MEASURE_PERIOD);
  }
  return 0;
}


//-----------------------------------------------------------------------------
void setup () {
  
    #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[temperatureMote - ");
        Serial.print(VERSION);
        Serial.println("]");
    #endif
    
    setup_radio();
    
    chBegin(mainThread);
}

//-----------------------------------------------------------------------------
void mainThread () {
  
  // start heartbeat thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 1, HeartbeatThread, NULL);
  
  // start temperature thread
  chThdCreateStatic(waThread2, sizeof(waThread2),
                    NORMALPRIO + 2, TemperatureThread, NULL);

  while (true)
    loop();
}

//-----------------------------------------------------------------------------
void loop () {
  
  // send report to gateway if ready
  noInterrupts();
  if (temperature.isReportReady()) {
    sendReadingReport();
  }
  interrupts();

  // bridge rf to serial messages
  noInterrupts();
  if (radio.receiveDone()) {
      if (radio.DATALEN != sizeof(Message)) {
          Serial.print("Invalid payload received, not matching Payload struct!");
      } else {
          memcpy(&inbound, (byte*) radio.DATA, sizeof inbound);
          consumeRf();
      }
      if (radio.ACK_REQUESTED) {
          radio.sendACK();
      }
  }
  interrupts();
  
}

//------------------------------------------------------------------------------
// consume inbound RF message
//
static void consumeRf() {
}

//------------------------------------------------------------------------------
// Dispatches the mote reading report over RF wireless
//
static void sendReadingReport() {
    memset(&outbound, 0, sizeof(outbound));
    
    outbound.msg.direction = RF_TO_MQTT  ;
    outbound.msg.type = MSG_READING;
    outbound.msg.source = NODEID;
    outbound.msg.destination = 0;
    outbound.msg.rssi = 0;
    
    SensorData* report = temperature.report();
    memcpy(&outbound.msg.data, report, sizeof(*report));
    
    #if DEBUG
        Serial.print("Broadcasting report to gateway...");
    #endif
    if (radio.sendWithRetry(GATEWAYID, outbound.raw, MSG_LENGTH)) {
        #if DEBUG
            Serial.println("ACK");
        #endif
    } else {
        #if DEBUG
            Serial.println("No ACK!");
        #endif
    }  
}

//------------------------------------------------------------------------------
// Initializes the RF radio.
//
static void setup_radio() {
    #if DEBUG
        Serial.print("setup radio...");
    #endif
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.setHighPower();
    delay(1000);
    #if DEBUG
        Serial.println("ok!");
    #endif
}

