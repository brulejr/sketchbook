/* -----------------------------------------------------------------------------
   Freezer Monitor
  
   Monitors several sensors on a freezer watching for out-of-ordinary behavior.
 
   Circuit:
   * RFM12B attached to digital pins 10, 11, 12, 13
   * Battery voltage monitor attached to analog pin 0
   * LDR sensor attached to analog pin 1
   * OneWire bus attached to digital pin 3
   * Hall Effect sensor attached to digital pin 7
   * Sensor Status LED attached to digital pin 8
   * RF Status LED attached to digital pin 9
 
   Created 19-JAN-2014 by Jon Brule
----------------------------------------------------------------------------- */

#include <JeeLib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/sleep.h>

#define VERSION "v0.1"

#define SERIAL     1     // set to 1 to also report readings on the serial port
#define DEBUG      1     // set to 1 to display each loop() run
#define BAUD_RATE  57600

#define APIN_LDR        1  // data connection for photocell
#define DPIN_ONEWIRE    3  // data connection for the Dallas OneWire bus
#define DPIN_HALL       7  // data connection for hall effect
#define DPIN_RF_LED     9  // led indicating rf traffic
#define LED_DELAY       500

#define MEASURE_PERIOD  100 // how often to measure, in tenths of seconds
#define RETRY_PERIOD    10  // how soon to retry if ACK didn't come in
#define RETRY_LIMIT     5   // maximum number of times to retry
#define ACK_TIME        10  // number of milliseconds to wait for an ack
#define REPORT_EVERY    2   // report every N measurement cycles
#define SMOOTH          3   // smoothing factor used for running averages

#define NODEID          2       // node ID used for this unit
#define NETWORKID       99
#define FREQUENCY       RF12_915MHZ

#define RAW_LENGTH      16
#define FREEZER_ZONE    0x01
#define DATA_TYPE       0x02

// set the sync mode to 2 if the fuses are still the Arduino default
// mode 3 (full powerdown) can only be used with 258 CK startup fuses
#define RADIO_SYNC_MODE 2

enum { MEASURE, REPORT, TASK_END };

Scheduler scheduler(TASK_END);

static byte reportCount;    // count up until next report, i.e. packet send

struct SensorData {
    byte zone;
    byte type;
    byte network;
    byte node;
    union {
        byte raw[RAW_LENGTH];
        struct {
            byte light;   // light sensor: 0..255
            byte door;    // door sensor: 0..1
            int tempIn;   // temperature: -500..+500 (tenths)
            int tempOut;  // temperature: -500..+500 (tenths)
        };
    };
};

// dallas onewire configuration
OneWire oneWire(DPIN_ONEWIRE);
DallasTemperature oneWireSensors(&oneWire);
DeviceAddress thermometerInside  = { 0x28, 0xC3, 0x42, 0x77, 0x03, 0x00, 0x00, 0x78 };
DeviceAddress thermometerOutside  = { 0x10, 0x74, 0x6C, 0x53, 0x02, 0x08, 0x00, 0xE6 };

// container for sensor data
SensorData sensorData;

// has to be defined because we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  
    #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[freezerNode - ");
        Serial.print(VERSION);
        Serial.println("]");
        serialFlush();
    #endif
  
    setup_sensors();
  
    pinMode(DPIN_RF_LED, OUTPUT);

    rf12_initialize(NODEID, FREQUENCY, NETWORKID);
    rf12_sleep(RF12_SLEEP);
  
    reportCount = REPORT_EVERY;     // report right away for easy debugging
    scheduler.timer(MEASURE, 0);    // start the measurement loop going
  
    sensorData.zone = FREEZER_ZONE;
    sensorData.type = DATA_TYPE;
    sensorData.network = NETWORKID;
    sensorData.node = NODEID;
    for (int i = 0; i< RAW_LENGTH; i++) {
        sensorData.raw[i] = 0x00;
    }
}


//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  
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
    
}

//------------------------------------------------------------------------------
// blinks the give led pin
//
static void blink_led(byte pin, int delay_ms) {
    digitalWrite(pin, HIGH);
    delay(delay_ms);
    digitalWrite(pin, LOW);
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
  
}

//------------------------------------------------------------------------------
// generate periodic report (i.e. send out a packet and optionally report on 
// serial port)
//
static void doReport() {
  
    rf12_sleep(RF12_WAKEUP);
    rf12_sendNow(0, &sensorData, sizeof sensorData);
    rf12_sendWait(RADIO_SYNC_MODE);
    rf12_sleep(RF12_SLEEP);
    blink_led(DPIN_RF_LED, LED_DELAY);

    #if SERIAL
        Serial.print("FREEZER ");
        Serial.print((int) sensorData.light);
        Serial.print(' ');
        Serial.print((int) sensorData.door);
        Serial.print(' ');
        Serial.print((int) sensorData.tempIn);
        Serial.print(' ');
        Serial.print((int) sensorData.tempOut);
        Serial.println();
        serialFlush();
    #endif
    
}

//------------------------------------------------------------------------------
// Initializes the sensors.
//
static void setup_sensors() {
    pinMode(APIN_LDR, INPUT);
    pinMode(DPIN_HALL, INPUT);
    oneWireSensors.begin();
}

