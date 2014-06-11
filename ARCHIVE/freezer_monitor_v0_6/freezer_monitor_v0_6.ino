/* -----------------------------------------------------------------------------
   Freezer Monitor
  
   Accepts item numbers from PS/2 connection and submits initiates an Item
   process via a RESTful web service.
 
   Circuit:
   * RFM12B attached to pins 10, 11, 12, 13
   * PS/2 keyboard attached to pins 3 (clock), 4 (data)
   * Status LEDs attached to pins 7 (red), 8 (yellow), 9 (green)
 
   Created 18-JAN-2014 by Jon Brule
----------------------------------------------------------------------------- */

#include <RFM12B.h>

#include <OneWire.h>
#include <DallasTemperature.h> 

#include <TimerOne.h>


#define VERSION "v0.6"

#define DPIN_ONEWIRE 3  // data connection for the Dallas OneWire bus
#define DPIN_HALL    8  // data connection for hall effect
#define APIN_LDR     0  // data connection for photocell

#define LED_STATUS 9

#define LOOP_DELAY 0

#define MYID       101
#define NETWORKID  100
#define GATEWAYID  1
#define FREQUENCY  RF12_915MHZ
#define KEY        "ABCDABCDABCDABCD"
#define ACK_TIME   50

#define PAYLOAD_SIZE 16
#define CMD_BOOT     0x10
#define CMD_READING  0x11


struct SensorData {
  boolean available;
  unsigned int ldr;
  unsigned int hall;
  float tempInsideC;
  float tempOutsideC;
};

struct SensorThresholds {
  unsigned int light;
  unsigned int door;
  float minTempInsideC;
  float maxTempInsideC;
  float minTempOutsideC;
  float maxTempOutsideC;
};

// dallas onewire configuration
OneWire oneWire(DPIN_ONEWIRE);
DallasTemperature oneWireSensors(&oneWire);
DeviceAddress thermometerInside  = { 0x10, 0x9B, 0x96, 0x53, 0x02, 0x08, 0x00, 0xF0 };
DeviceAddress thermometerOutside  = { 0x10, 0x74, 0x6C, 0x53, 0x02, 0x08, 0x00, 0xE6 };

// container for sensor data
SensorData sensorData;
SensorThresholds sensorThresholds;

// rfm12b radio
RFM12B radio;
char payload[PAYLOAD_SIZE];


/******************************************************************************
 * Main Arduino Subroutines
 ******************************************************************************/

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  
  sensorData.available = false;
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
  Serial.print("RF Freezer Monitor - ");
  Serial.println(VERSION);
  
  // sensor configurations
  pinMode(APIN_LDR, INPUT);
  pinMode(DPIN_HALL, INPUT);

  // output configuration
  pinMode(LED_STATUS, OUTPUT);
  
  // radio initialization
  radio.Initialize(MYID, FREQUENCY, NETWORKID, 0);
  radio.Encrypt((byte*)KEY);
  radio.Sleep();
  
  // start up the one-wire sensors
  oneWireSensors.begin();
  
  // define default sensor thresholds
  setup_sensor_thresholds();
  
  // setup background timer to interogate sensors  
  Timer1.initialize(1000000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt(interogate_sensors); // attach the service routine here
  
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  delay(LOOP_DELAY);
}


/******************************************************************************
 * Internal Subroutines
 ******************************************************************************/

//------------------------------------------------------------------------------
int check_status() {
  int status = LOW;
  if (sensorData.ldr > sensorThresholds.light) {
    status = HIGH;
  }
  if (sensorData.hall == sensorThresholds.door) {
    status = HIGH;
  }
  if (sensorData.tempInsideC < sensorThresholds.minTempInsideC) {
    status = HIGH;
  }
  if (sensorData.tempInsideC > sensorThresholds.maxTempInsideC) {
    status = HIGH;
  }
  return status;
}

//------------------------------------------------------------------------------
void interogate_sensors() {

  // block access to sensor data  
  sensorData.available = false;
  
  // read light level
  sensorData.ldr = map(analogRead(APIN_LDR), 0, 1023, 0, 255);

  // read door status
  sensorData.hall = digitalRead(DPIN_HALL);
  
  // read one-wire sensors
  oneWireSensors.requestTemperatures();
  sensorData.tempInsideC = oneWireSensors.getTempC(thermometerInside);
  sensorData.tempOutsideC = oneWireSensors.getTempC(thermometerOutside);

  // enable access to sensor data  
  sensorData.available = true;
 
  update_status(); 
  
}

//------------------------------------------------------------------------------
void setup_sensor_thresholds() {
  payload[0] = MYID;
  payload[1] = CMD_BOOT;
  radio.Wakeup();
  radio.Send(GATEWAYID, payload, 2, true);
  if (waitForAck(GATEWAYID)) {
    Serial.print("ok!");
    sensorThresholds.light = 50;
    sensorThresholds.door = HIGH;
    sensorThresholds.minTempInsideC = 0.00;
    sensorThresholds.maxTempInsideC = 2.00;
    sensorThresholds.minTempOutsideC = 19.00;
    sensorThresholds.maxTempOutsideC = 20.00;
  } else {
    Serial.println("No RF Gateway - Using factory sensor defaults...");
    sensorThresholds.light = 50;
    sensorThresholds.door = HIGH;
    sensorThresholds.minTempInsideC = 0.00;
    sensorThresholds.maxTempInsideC = 2.00;
    sensorThresholds.minTempOutsideC = 19.00;
    sensorThresholds.maxTempOutsideC = 20.00;
  }
}

//------------------------------------------------------------------------------
void update_status() {
  digitalWrite(LED_STATUS, check_status());
}

//------------------------------------------------------------------------------
// wait a few milliseconds for proper ACK to me, return true if indeed received
static bool waitForAck(byte theNodeID) {
  long now = millis();
  while (millis() - now <= ACK_TIME) {
    if (radio.ACKReceived(theNodeID))
      return true;
  }
  return false;
}
