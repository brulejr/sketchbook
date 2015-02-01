/* -----------------------------------------------------------------------------
   Test Mote Sensor
  
   Monitors several sensors and offeres several controls.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * Battery voltage monitor attached to analog pin 0
   * Temperature sensor attached to analog pin 1
   * Dimmer LED attached to digital pin 6
   * Switch LED attached to digital pin 8
   * RF Status LED attached to digital pin 9
 
   Created 01-FEB-2015 by Jon Brule
----------------------------------------------------------------------------- */
#include <ChibiOS_AVR.h>
#include <Heartbeat.h>
#include <RFM69.h>
#include <SPI.h>
#include <Message.h>
#include "Reading.h"

#define VERSION    "v0.1"

#define SERIAL      1   // set to 1 to also report readings on the serial port
#define DEBUG       1   // set to 1 to display each loop() run
#define BAUD_RATE   57600

#define APIN_BATTERY        0
#define APIN_TEMPERATURE    1
#define DPIN_DIMMER         6
#define DPIN_TOGGLE         8
#define DPIN_MOTE_LED       9  // moteinos have LEDs on D9

#define VOLTAGE             3.3

#define COMPONENT_TOGGLE    1
#define COMPONENT_DIMMER    2

#define COMMAND_INCREASE    1
#define COMMAND_DECREASE    2
#define COMMAND_PERCENTAGE  3

#define MEASURE_PERIOD      1000
#define REPORT_PERIOD       5000
#define SMOOTHING_FACTOR    3
#define BATTERY_CYCLE       5
#define DIMMER_CYCLE        1
#define TEMP_CYCLE          5
#define TOGGLE_CYCLE        1

#define DIMMER_INTERVAL     15

#define NODEID        3   // unique for each node on same network
#define GATEWAYID     1
#define NETWORKID     99  // same for all nodes that talk to each other
#define FREQUENCY     RF69_915MHZ

Reading battery(DIMMER_CYCLE, DPIN_DIMMER, SMOOTHING_FACTOR, readDimmer, saveDimmer);
Reading dimmer(BATTERY_CYCLE, APIN_BATTERY, SMOOTHING_FACTOR, readBattery, saveBattery);
Reading temp(TEMP_CYCLE, APIN_TEMPERATURE, SMOOTHING_FACTOR, readTemp, saveTemp);
Reading toggle(TOGGLE_CYCLE, DPIN_TOGGLE, readToggle, saveToggle);

RFM69 radio;

struct SensorData {
    byte tempInC;  // temperature sensor: C * 10
    byte battery;  // battery voltage
    byte toggle;   // toggle (0 or 1)
    byte dimmer;   // dimmer
} sensorData;
boolean haveReadings = false;

int dimmerState = 0;

Message inbound, outbound;


//----------------------------------------------------------------------------- 
// heartbeat thread
//
static WORKING_AREA(waThread1, 50);
static msg_t ReportThread(void *arg) {
  while (1) {
    chThdSleepMilliseconds(REPORT_PERIOD);
    haveReadings = true;
  }
  return 0;
}

//----------------------------------------------------------------------------- 
// temperature thread
//
static WORKING_AREA(waThread2, 50);
static msg_t ReadingThread(void *arg) {
  while (1) {
    temp.measure();
    toggle.measure();
    dimmer.measure();
    battery.measure();
    chThdSleepMilliseconds(MEASURE_PERIOD);
  }
  return 0;
}


//-----------------------------------------------------------------------------
void setup () {
  
    #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[testMote - ");
        Serial.print(VERSION);
        Serial.println("]");
    #endif
    
    setup_radio();
    setup_sensors();
    
    chBegin(mainThread);
}

//------------------------------------------------------------------------------
// Initializes the RF radio.
//
void setup_radio() {
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


//------------------------------------------------------------------------------
// Initializes the sensors.
//
void setup_sensors() {
  #if DEBUG
    Serial.print("setup sensors...");
  #endif
  
  pinMode(DPIN_MOTE_LED, OUTPUT);
  
  pinMode(DPIN_TOGGLE, OUTPUT);
  digitalWrite(DPIN_TOGGLE, LOW);

  pinMode(DPIN_DIMMER, OUTPUT);
  analogWrite(DPIN_DIMMER, dimmerState);
  
  memset(&sensorData, 0, sizeof(sensorData));
  
  #if DEBUG
    Serial.println("ok!");
  #endif
}

//-----------------------------------------------------------------------------
void mainThread () {
  
  // start report thread
  chThdCreateStatic(waThread1, sizeof(waThread1),
                    NORMALPRIO + 1, ReportThread, NULL);
  
  // start temperature thread
  chThdCreateStatic(waThread2, sizeof(waThread2),
                    NORMALPRIO + 2, ReadingThread, NULL);
                      
  while (true)
    loop();
}

//-----------------------------------------------------------------------------
void loop () {
  
  // send report to gateway if ready
  noInterrupts();
  if (haveReadings) {
    sendReadingReport();
    haveReadings = false;
  }
  interrupts();

  // recevie rf messages
  noInterrupts();
  if (radio.receiveDone()) {
      Serial.println("received rf message");
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
void consumeRf() {
  if ((inbound.msg.type == MSG_COMMAND) && (inbound.msg.destination == NODEID)) {
    if (inbound.msg.component == COMPONENT_TOGGLE) {
      digitalWrite(DPIN_TOGGLE, inbound.msg.data[0]);
      toggle.measure();
      haveReadings = true;
    } else if (inbound.msg.component == COMPONENT_DIMMER) {
      if (inbound.msg.data[0] == COMMAND_INCREASE) {
        dimmerState += DIMMER_INTERVAL;
        dimmerState = (dimmerState > 255) ? 255 : dimmerState;
      } else if (inbound.msg.data[0] == COMMAND_DECREASE) {
        dimmerState -= DIMMER_INTERVAL;
        dimmerState = (dimmerState < 0) ? 0 : dimmerState;
      } else if (inbound.msg.data[0] == COMMAND_PERCENTAGE) {
        dimmerState = inbound.msg.data[1];
      }
      analogWrite(DPIN_DIMMER, dimmerState);
      dimmer.measure();
      haveReadings = true;
    }
  }
}

//------------------------------------------------------------------------------
// Dispatches the mote reading report over RF wireless
//
void sendReadingReport() {
  
    digitalWrite(DPIN_MOTE_LED, HIGH);
    
    memset(&outbound, 0, sizeof(outbound));
    outbound.msg.type = MSG_READING;
    outbound.msg.source = NODEID;
    outbound.msg.destination = 0;
    outbound.msg.component = 0;
    outbound.msg.rssi = 0;
    memcpy(&outbound.msg.data, &sensorData, sizeof(sensorData));
    
    #if DEBUG
        Serial.print("Reading<temperature = ");
        Serial.print(sensorData.tempInC);
        Serial.print(", toggle = ");
        Serial.print(sensorData.toggle);
        Serial.print(", dimmer = ");
        Serial.print(sensorData.dimmer);
        Serial.print(", battery = ");
        Serial.print(sensorData.battery);
        Serial.println(">");
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

    digitalWrite(DPIN_MOTE_LED, LOW);
}

//-----------------------------------------------------------------------------
int readBattery(byte pin) {
    int val = analogRead(pin);
    return int(((val/255) * VOLTAGE) * 10);
}

//-----------------------------------------------------------------------------
int readDimmer(byte pin) {
    return dimmerState;
}

//-----------------------------------------------------------------------------
int readTemp(byte pin) {
    int tempRaw = analogRead(pin);
    float tempVolts = (((float)tempRaw / 1024) * VOLTAGE);
    return int((tempVolts - 0.5) / 0.01);
}

//-----------------------------------------------------------------------------
int readToggle(byte pin) {
    return digitalRead(pin);
}

//-----------------------------------------------------------------------------
void saveBattery(int reading) {
    sensorData.battery = (byte) reading;
}

//-----------------------------------------------------------------------------
void saveDimmer(int reading) {
    sensorData.dimmer = (byte) reading;
}

//-----------------------------------------------------------------------------
void saveTemp(int reading) {
    sensorData.tempInC = (byte) reading;
}

//-----------------------------------------------------------------------------
void saveToggle(int reading) {
    sensorData.toggle = (byte) reading;
}

