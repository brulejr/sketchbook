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
#include <avr/sleep.h>
#include <avr/power.h>
#include <Message.h>
#include <RFM69.h>
#include <SPI.h>
#include "Sensors.h"

#define VERSION        "v0.1"

#define SERIAL         1   // set to 1 to also report readings on the serial port
#define DEBUG          1   // set to 1 to display each loop() run
#define BAUD_RATE      57600

#define REPORT_PERIOD  1000 * 1000

#define NODEID         5   // unique for each node on same network
#define GATEWAYID      1
#define NETWORKID      99  // same for all nodes that talk to each other
#define FREQUENCY      RF69_915MHZ

#define MOTE_LED_PIN   9  // moteinos have LEDs on D9


volatile int f_timer = 0;

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
    setupSleep();
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
    
    
}

void setupSleep() {
    // normal timer operation
    TCCR1A = 0x00;
    
    // clear the timer counter register
    TCNT1=0x0000;
    
    // configure the prescaler for 1:1024, a 4.09 sec timeout
    TCCR1B = 0x05;
    
    // enable the timer overlow interrupt
    TIMSK1=0x01;
}


//-----------------------------------------------------------------------------
// Main Loop
//
void loop () {
    if (f_timer == 1) {
        f_timer = 0;
        enterSleep();
    }
    
}

//-----------------------------------------------------------------------------
// Timer1 Interrupt Service Routine
ISR(TIMER1_OVF_vect) {
     if (f_timer == 0) {
        f_timer = 1;
        
        noInterrupts();
        digitalWrite(MOTE_LED_PIN, HIGH);        
        sensors->measure();
        report();
        digitalWrite(MOTE_LED_PIN, LOW);
        interrupts();         
        
     }
}

//-----------------------------------------------------------------------------
// sleeps the microcontroller until the internal timer expires
void enterSleep(void) {
  set_sleep_mode(SLEEP_MODE_IDLE);
  
  sleep_enable();

  // Disable all of the unused peripherals. This will reduce power
  // consumption further and, more importantly, some of these
  // peripherals may generate interrupts that will wake our Arduino from
  // sleep!
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  

  // Now enter sleep mode
  sleep_mode();
  
  // NOTE: the program will continue from here after the timer timeout
  
  // disable sleep
  sleep_disable();
  
  // re-enable the peripherals
  power_all_enable();
}

//-----------------------------------------------------------------------------
// generates a sensor report, sending it to the RF gateway
void report() {    
    sensors->report(sensorData);
    
    memset(&outbound, 0, sizeof(outbound));
    outbound.msg.type = MSG_READING;
    outbound.msg.source = NODEID;
    outbound.msg.destination = 0;
    outbound.msg.component = 0;
    outbound.msg.rssi = 0;
    memcpy(&outbound.msg.data, sensorData, sizeof(*sensorData));
    
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

