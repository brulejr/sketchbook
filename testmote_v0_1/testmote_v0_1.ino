/* -----------------------------------------------------------------------------
   Test Mote Sensor
  
   Monitors several sensors and offeres several controls.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * Battery voltage monitor attached to analog pin 0
   * Temperature sensor attached to digital pin 3
   * RF Status LED attached to digital pin 9
 
   Created 20-JAN-2015 by Jon Brule
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

#define APIN_BATTERY      0
#define APIN_TEMPERATURE  1
#define DPIN_LIGHT        9
#define DPIN_MOTE_LED     12  // moteinos have LEDs on D9

#define VOLTAGE           3.3

#define MEASURE_PERIOD    1000
#define REPORT_PERIOD     5000
#define SMOOTHING_FACTOR  3
#define BATTERY_CYCLE     10
#define LIGHT_CYCLE       1
#define TEMP_CYCLE        5

#define NODEID        3   // unique for each node on same network
#define GATEWAYID     1
#define NETWORKID     99  // same for all nodes that talk to each other
#define FREQUENCY     RF69_915MHZ

Reading battery(BATTERY_CYCLE, APIN_BATTERY, SMOOTHING_FACTOR, readBattery, saveBattery);
Reading light(LIGHT_CYCLE, DPIN_LIGHT, NO_SMOOTHING, readLight, saveLight);
Reading temp(TEMP_CYCLE, APIN_TEMPERATURE, SMOOTHING_FACTOR, readTemp, saveTemp);

RFM69 radio;

struct SensorData {
    byte tempInC;  // temperature sensor: C * 10
    byte battery;  // battery voltage
    byte light;    // light (0 or 1)
} sensorData;
boolean haveReadings = false;

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
    light.measure();
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
    
    pinMode(DPIN_MOTE_LED, OUTPUT);
    pinMode(DPIN_LIGHT, OUTPUT);
    digitalWrite(DPIN_LIGHT, HIGH);
    
    memset(&sensorData, 0, sizeof(sensorData));
    
    chBegin(mainThread);
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

}

//------------------------------------------------------------------------------
// Dispatches the mote reading report over RF wireless
//
static void sendReadingReport() {
  
    digitalWrite(DPIN_MOTE_LED, HIGH);
    
    memset(&outbound, 0, sizeof(outbound));
    outbound.msg.type = MSG_READING;
    outbound.msg.source = NODEID;
    outbound.msg.destination = 0;
    outbound.msg.rssi = 0;
    memcpy(&outbound.msg.data, &sensorData, sizeof(sensorData));
    
    #if DEBUG
        Serial.print("Reading<temperature = ");
        Serial.print(sensorData.tempInC);
        Serial.print(", light = ");
        Serial.print(sensorData.light);
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
int readLight(byte pin) {
    return digitalRead(pin);
}

//-----------------------------------------------------------------------------
int readTemp(byte pin) {
    int tempRaw = analogRead(pin);
    float tempVolts = (((float)tempRaw / 1024) * 3.3);
    return int((tempVolts - 0.5) / 0.01);
}

//-----------------------------------------------------------------------------
void saveBattery(int reading) {
    sensorData.battery = (byte) reading;
}

//-----------------------------------------------------------------------------
void saveLight(int reading) {
    sensorData.light = (byte) reading;
}

//-----------------------------------------------------------------------------
void saveTemp(int reading) {
    sensorData.tempInC = (byte) reading;
}

