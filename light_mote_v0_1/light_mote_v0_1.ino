/* -----------------------------------------------------------------------------
   Light Sensor
  
   Monitors several sensors on a light watching for out-of-ordinary behavior.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * Battery voltage monitor attached to analog pin 0
   * Momentary button attached to digital pin 2
   * LED Light attached to digital pin 3
   * RF Status LED attached to digital pin 9
 
   Created 08-JUN-2014 by Jon Brule
----------------------------------------------------------------------------- */
#include <ChibiOS_AVR.h>
#include <Heartbeat.h>
#include <RFM69.h>
#include <SPI.h>
#include <Event.h>
#include "Light.h"

#define VERSION    "v0.2"

#define SERIAL      1   // set to 1 to also report readings on the serial port
#define DEBUG       1   // set to 1 to display each loop() run
#define BAUD_RATE   57600

#define APIN_BATTERY     0
#define DPIN_BUTTON      2
#define DPIN_LIGHT       3
#define DPIN_MOTE_LED    9  // moteinos have LEDs on D9

#define MEASURE_PERIOD   1000
#define REPORT_INTERVAL  10

#define NODEID        3   // unique for each node on same network
#define GATEWAYID     1
#define NETWORKID     99  // same for all nodes that talk to each other
#define FREQUENCY     RF69_915MHZ

#define CMD_ON        21
#define CMD_OFF       22
#define CMD_TOGGLE    20

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 100;    // the debounce time; increase if the output flickers

Light light(REPORT_INTERVAL, DPIN_LIGHT, APIN_BATTERY);
RFM69 radio;

EventMessage inbound, outbound;


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
// light thread
//
static WORKING_AREA(waThread2, 50);
static msg_t LightThread(void *arg) {
  while (1) {
    light.measure();
    chThdSleepMilliseconds(MEASURE_PERIOD);
  }
  return 0;
}


//-----------------------------------------------------------------------------
void setup () {
  
    #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[lightMote - ");
        Serial.print(VERSION);
        Serial.println("]");
    #endif
    
    pinMode(DPIN_BUTTON, INPUT);
    
    setup_radio();
    
    chBegin(mainThread);
}

//-----------------------------------------------------------------------------
void mainThread () {
  
  // start heartbeat thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 1, HeartbeatThread, NULL);
  
  // start light thread
  chThdCreateStatic(waThread2, sizeof(waThread2),
                    NORMALPRIO + 2, LightThread, NULL);

  while (true)
    loop();
}

//-----------------------------------------------------------------------------
void loop () {
  
  // send report to gateway if ready
  noInterrupts();
  if (light.isReportReady()) {
    sendReadingReport();
  }
  interrupts();

  // process light control buttons    
  noInterrupts();
  handleLightButton();
  interrupts();
  
  // bridge rf to serial messages
  noInterrupts();
  if (radio.receiveDone()) {
      if (radio.DATALEN != sizeof(EventMessage)) {
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
  EventRecord event = inbound.event;
  if (event.type == EVENT_COMMAND) {
    Serial.print("command=[");
    Serial.print(inbound.event.data[0], DEC);
    Serial.println("]");
    if (event.data[0] == CMD_ON) {
      light.on();
    } else if (event.data[0] == CMD_OFF) {
      light.off();
    } else if (event.data[0] == CMD_TOGGLE) {
      light.toggle();
    }
  }
}

//------------------------------------------------------------------------------
// Process physical button push that toggles the light state
//
static void handleLightButton() {
  int reading = digitalRead(DPIN_BUTTON);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  } 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        light.toggle();
      }
    }
  }
  lastButtonState = reading;
}

//------------------------------------------------------------------------------
// Dispatches the mote reading report over RF wireless
//
static void sendReadingReport() {
    memset(&outbound, 0, sizeof(outbound));
    outbound.event.type = EVENT_READING;
    outbound.event.network = NETWORKID;
    outbound.event.source = NODEID;
    
    SensorData* report = light.report();
    memcpy(&outbound.event.data, report, sizeof(*report));
    
    #if DEBUG
        Serial.print("Broadcasting report to gateway...");
    #endif
    if (radio.sendWithRetry(GATEWAYID, outbound.raw, EVENT_LENGTH)) {
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

