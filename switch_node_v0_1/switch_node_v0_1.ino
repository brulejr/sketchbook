/* -----------------------------------------------------------------------------
   Switch Node
  
   Provides control and monitoring of an electrical switch.
 
   Circuit:
   * RFM12B attached to digital pins 10, 11, 12, 13
   * Battery voltage monitor attached to analog pin 0
   * Switch connected to digital pin 3
   * RF Status LED attached to digital pin 9
 
   Created 28-MAY-2014 by Jon Brule
----------------------------------------------------------------------------- */

#include <JeeLib.h>
#include <avr/sleep.h>

#define VERSION "v0.1"

#define SERIAL     1     // set to 1 to also report readings on the serial port
#define DEBUG      1     // set to 1 to display each loop() run
#define BAUD_RATE  57600

#define APIN_BATTERY    0     // data connection for battery
#define DPIN_CTRL_LED   3     // LED simulating the swtch
#define DPIN_RF_LED     9     // led indicating rf traffic
#define LED_DELAY       500
#define VDD_RATIO       1.622

#define MEASURE_PERIOD  100   // how often to measure, in tenths of seconds
#define RETRY_PERIOD    10    // how soon to retry if ACK didn't come in
#define RETRY_LIMIT     5     // maximum number of times to retry
#define ACK_TIME        10    // number of milliseconds to wait for an ack
#define REPORT_EVERY    2     // report every N measurement cycles
#define SMOOTH          3     // smoothing factor used for running averages

#define NODEID          3     // node ID used for this unit
#define NETWORKID       99
#define FREQUENCY       RF12_915MHZ

#define RAW_LENGTH      16
#define SWITCH_ZONE     0x02
#define CMD_TYPE        0x01
#define DATA_TYPE       0x02

#define CMD_ON          0x01
#define CMD_OFF         0x02

// set the sync mode to 2 if the fuses are still the Arduino default
// mode 3 (full powerdown) can only be used with 258 CK startup fuses
#define RADIO_SYNC_MODE 2

enum { MEASURE, REPORT, TASK_END };

Scheduler scheduler(TASK_END);

static byte reportCount;    // count up until next report, i.e. packet send

// container for command data
struct CommandData {
    union {
        byte raw[RAW_LENGTH];
        struct {
            byte type;   // command
            byte state;  // swtich state: 0 or 1
        };
    };
};
CommandData cmdData;

// container for sensor data
struct SensorData {
    byte zone;
    byte type;
    byte network;
    byte node;
    union {
        byte raw[RAW_LENGTH];
        struct {
            byte switchState;   // switch state: 0 or 1
            int battery;        // battery voltage
        };
    };
};
SensorData sensorData;

// has to be defined because we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

byte firstTime = true;

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  
    #if SERIAL || DEBUG
        Serial.begin(BAUD_RATE);
        Serial.print("\n[switchNode - ");
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
  
    sensorData.zone = SWITCH_ZONE;
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
    
    if (rf12_recvDone() && rf12_crc == 0) {
        for (byte i = 0; i < rf12_len; ++i) {
            if (i < RAW_LENGTH) {
                cmdData.raw[i] = rf12_data[i];
            }
        }
        handleCommand();
    }
    
}

//------------------------------------------------------------------------------
// blinks the rf led pin
//
static void blink_rf_led(byte pin, int delay_ms) {
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
//
//
static void handleCommand() {
    switch (cmdData.type) {
        case CMD_ON:
            digitalWrite(DPIN_CTRL_LED, HIGH);
            break;
        case CMD_OFF:
            digitalWrite(DPIN_CTRL_LED, LOW);
            break;
    }
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
  
    // read switch status
    sensorData.switchState = digitalRead(DPIN_CTRL_LED);
  
    // read battery voltage
    int val = analogRead(APIN_BATTERY);
    int battery = int(val * 0.00488 * VDD_RATIO * 10);
    sensorData.battery = smoothedAverage(sensorData.battery, battery, firstTime);
  
    firstTime = false;
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
    blink_rf_led(DPIN_RF_LED, LED_DELAY);

    #if SERIAL
        Serial.print("SWITCH ");
        Serial.print((int) sensorData.switchState);
        Serial.print(' ');
        Serial.print((int) sensorData.battery);
        Serial.println();
        serialFlush();
    #endif
    
}

//------------------------------------------------------------------------------
// Initializes the sensors.
//
static void setup_sensors() {
    pinMode(DPIN_CTRL_LED, INPUT);
    pinMode(DPIN_RF_LED, INPUT);
}

