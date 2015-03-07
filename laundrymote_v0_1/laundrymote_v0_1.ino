/* -----------------------------------------------------------------------------
   Laundry Mote Sensor
  
   Monitors a washer and dryer.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * A0 - Battery voltage monitor
   * A1 - Light detector
   * A2 - Temperature
   * D2 - Water Leak detector
 
   Created 05-MAR-2015 by Jon Brule
----------------------------------------------------------------------------- */
#include <Message.h>
#include <RFM69.h>
#include <SPI.h>
#include <TimerOne.h>
#include "Sensors.h"

#define VERSION        "v0.1"

#define SERIAL         1   // set to 1 to also report readings on the serial port
#define DEBUG          1   // set to 1 to display each loop() run
#define BAUD_RATE      57600

#define REPORT_PERIOD  1000 * 1000

#define NODEID         3   // unique for each node on same network
#define GATEWAYID      1
#define NETWORKID      99  // same for all nodes that talk to each other
#define FREQUENCY      RF69_915MHZ

#define MOTE_LED_PIN   9  // moteinos have LEDs on D9


Sensors* sensors;
SensorData* sensorData;

RFM69 radio;
Message inbound, outbound;


//-----------------------------------------------------------------------------
// Initialization
//
void setup () {
    #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[laundryMote - ");
        Serial.print(VERSION);
        Serial.println("]");
    #endif
    setupRadio();
    setupSensors();
}

void setupRadio() {
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

void setupSensors() {
    sensors = new Sensors();
    sensorData = new SensorData();
    memset(sensorData, 0, sizeof(*sensorData));
    
    Timer1.initialize(REPORT_PERIOD);
    Timer1.attachInterrupt(report);
}


//-----------------------------------------------------------------------------
// Main Loop
//
void loop () {
  
    noInterrupts();
    sensors->measure();
    interrupts();
    
}

//-----------------------------------------------------------------------------
void report() {
    digitalWrite(MOTE_LED_PIN, HIGH);
    
    sensors->report(sensorData);
    
    memset(&outbound, 0, sizeof(outbound));
    outbound.msg.type = MSG_READING;
    outbound.msg.source = NODEID;
    outbound.msg.destination = 0;
    outbound.msg.component = 0;
    outbound.msg.rssi = 0;
    memcpy(&outbound.msg.data, &sensorData, sizeof(sensorData));
    
    if (radio.sendWithRetry(GATEWAYID, outbound.raw, MSG_LENGTH)) {
        #if DEBUG
            Serial.println("ACK");
        #endif
    } else {
        #if DEBUG
            Serial.println("No ACK!");
        #endif
    }
    
    digitalWrite(MOTE_LED_PIN, LOW);
}

