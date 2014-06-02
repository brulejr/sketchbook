/* -----------------------------------------------------------------------------
   Freezer Sensor
  
   Monitors several sensors on a freezer watching for out-of-ordinary behavior.
 
   Circuit:
   * Moteino R4 w/ RFM69HW RF module
   * Battery voltage monitor attached to analog pin 0
   * LDR sensor attached to analog pin 1
   * OneWire bus attached to digital pin 3
   * Hall Effect sensor attached to digital pin 7
   * RF Status LED attached to digital pin 9
 
   Created 01-JUN-2014 by Jon Brule
----------------------------------------------------------------------------- */

#include <JeeLib.h>
#include <RFM69.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/sleep.h>

#define VERSION "v0.1"

#define SERIAL      1   // set to 1 to also report readings on the serial port
#define DEBUG       1   // set to 1 to display each loop() run
#define BAUD_RATE   57600

#define NODEID      2   // unique for each node on same network
#define GATEWAYID   1
#define NETWORKID   99  // same for all nodes that talk to each other
#define FREQUENCY   RF69_915MHZ

#define MSG_COMMAND 0x43
#define MSG_INFO    0x49
#define MSG_READING 0x52

#define ACK_TIME    30  // max # of ms to wait for an ack
#define LED         9   // Moteinos have LEDs on D9

#define RAW_LENGTH 16

#define APIN_BATTERY    0  // data connection for battery
#define APIN_LDR        1  // data connection for photocell
#define DPIN_ONEWIRE    3  // data connection for the Dallas OneWire bus
#define DPIN_HALL       7  // data connection for hall effect
#define DPIN_RF_LED     2  // rf activiyy indicator LED
#define DPIN_MOTE_LED   9  // moteinos have LEDs on D9
#define VOLTAGE         3.3

#define MEASURE_PERIOD  100
#define REPORT_EVERY    2
#define SMOOTH          3

RFM69 radio;

struct SensorData {
    byte msgtype;
    byte network;
    byte node;
    struct {
        byte light;   // light sensor: 0..255
        byte door;    // door sensor: 0..1
        int tempIn;   // temperature: -500..+500 (tenths)
        int tempOut;  // temperature: -500..+500 (tenths)
        int battery;  // battery voltage
    };
};

struct MessageData {
    union {
        byte raw[RAW_LENGTH];
        SensorData sensor;
    };
};

// container for sensor data
SensorData sensorData;
MessageData message;

// dallas onewire configuration
OneWire oneWire(DPIN_ONEWIRE);
DallasTemperature oneWireSensors(&oneWire);
DeviceAddress thermometerInside  = { 0x28, 0xC3, 0x42, 0x77, 0x03, 0x00, 0x00, 0x78 };
DeviceAddress thermometerOutside  = { 0x10, 0x74, 0x6C, 0x53, 0x02, 0x08, 0x00, 0xE6 };

enum { MEASURE, REPORT, TASK_END };
Scheduler scheduler(TASK_END);
static byte reportCount;

// has to be defined because we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }


//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  
    #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[freezerMote - ");
        Serial.print(VERSION);
        Serial.println("]");
        serialFlush();
    #endif
    
    setup_radio();
  
    setup_sensors();
    
    reportCount = REPORT_EVERY;     // report right away for easy debugging
    scheduler.timer(MEASURE, 0);    // start the measurement loop going
  
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  
    heartbeat(DPIN_MOTE_LED);
    
    switch (scheduler.pollWaiting()) {

        case MEASURE:
            // reschedule these measurements periodically
            scheduler.timer(MEASURE, MEASURE_PERIOD);
    
            doMeasure();

            // every so often, a report needs to be sent out
            if (++reportCount >= REPORT_EVERY) {
                reportCount = 0;
                scheduler.timer(REPORT, 0);
            }
            break;
            
        case REPORT:
            doReport();
            break;
    }
    
    if (radio.receiveDone()) {
        if (radio.ACK_REQUESTED) {
            radio.sendACK();
        }
    }
    
}

//------------------------------------------------------------------------------
// simulates a heartbeat to show activity
//
static void heartbeat(int pin) {
    int rate = 25;
    int pmw = 255;
    for(int i = 0; i < pmw; i++) {
        analogWrite(pin,i);
        delay(((60000/rate)*.1)/pmw);
    }
    for (int i = pmw; i > 0; i--){
        analogWrite(pin,i);
        delay(((60000/rate)*.2)/pmw);
    }
    for(int i = 0; i < pmw; i++) {
        analogWrite(pin,i);
        delay(((60000/rate)*.1)/pmw);
    }
    for (int i = pmw; i > 0; i--){
        analogWrite(pin,i);
        delay(((60000/rate)*.6)/pmw);
    }
}

//------------------------------------------------------------------------------
// flushes the serial port
//
static void serialFlush () {
    #if ARDUINO >= 100
        Serial.flush();
    #endif  
    delay(2); // make sure tx buf is empty before going back to sleep
}

//------------------------------------------------------------------------------
// utility code to perform simple smoothing as a running average
//
static int smoothedAverage(int prev, int next, byte firstTime =0) {
    return (firstTime) ? next : ((SMOOTH - 1) * prev + next + SMOOTH / 2) / SMOOTH;
}

//------------------------------------------------------------------------------
// readout all the sensors and other values
//
static void doMeasure() {
  
    Serial.println("doMeasure()");
  
    byte firstTime = sensorData.light == 0;
    
    // read light level
    byte light = map(analogRead(APIN_LDR), 0, 1023, 0, 255);
    sensorData.light = smoothedAverage(sensorData.light, light, firstTime);

    // read door status
    sensorData.door = digitalRead(DPIN_HALL);
  
    // read one-wire sensors
    oneWireSensors.requestTemperatures();
    int tempIn = int(oneWireSensors.getTempC(thermometerInside) * 10);
    sensorData.tempIn = smoothedAverage(sensorData.tempIn, tempIn, firstTime);
    int tempOut = int(oneWireSensors.getTempC(thermometerOutside) * 10);
    sensorData.tempOut = smoothedAverage(sensorData.tempOut, tempOut, firstTime);
    
    // read battery voltage
    int val = analogRead(APIN_BATTERY);
    int battery = int(((val/255) * VOLTAGE) * 10);
    sensorData.battery = smoothedAverage(sensorData.battery, battery, firstTime);
  
}

//------------------------------------------------------------------------------
// generate periodic report (i.e. send out a packet and optionally report on 
// serial port)
//
static void doReport() {
    
    digitalWrite(DPIN_RF_LED, HIGH);
    
    for (int i = 0; i< RAW_LENGTH; i++) {
        message.raw[i] = 0x00;
    }
    
    message.sensor = sensorData;
    message.sensor.msgtype = MSG_READING;
    message.sensor.network = NETWORKID;
    message.sensor.node = NODEID;
    if (radio.sendWithRetry(GATEWAYID, message.raw, RAW_LENGTH)) {
        Serial.print(" ok!");
    } else {
        Serial.print(" nothing...");
    }
    
    #if SERIAL
        Serial.print("FREEZER ");
        Serial.print((int) sensorData.light);
        Serial.print(' ');
        Serial.print((int) sensorData.door);
        Serial.print(' ');
        Serial.print((int) sensorData.tempIn);
        Serial.print(' ');
        Serial.print((int) sensorData.tempOut);
        Serial.print(' ');
        Serial.print((int) sensorData.battery);
        Serial.println();
        serialFlush();
    #endif

    digitalWrite(DPIN_RF_LED, LOW);
    
}

//------------------------------------------------------------------------------
// Initializes the sensors.
//
static void setup_radio() {
    #if DEBUG
        Serial.println("setup radio");
    #endif
    radio.initialize(FREQUENCY,NODEID,NETWORKID);
    radio.setHighPower();
}

//------------------------------------------------------------------------------
// Initializes the sensors.
//
static void setup_sensors() {
    #if DEBUG
        Serial.println("setup sensors");
    #endif
    
    pinMode(APIN_LDR, INPUT);
    pinMode(DPIN_HALL, INPUT);
    
    // configure one-wire bus
    oneWireSensors.begin();
    #if DEBUG
        Serial.print("Locating devices...");
        Serial.print("Found ");
        Serial.print(oneWireSensors.getDeviceCount(), DEC);
        Serial.println(" devices.");
        
        Serial.print("Parasite power is: "); 
        if (oneWireSensors.isParasitePowerMode()) Serial.println("ON");
        else Serial.println("OFF");
    #endif    
}
