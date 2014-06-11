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

#include <RFM12B.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimerOne.h>
#include <EmBencode.h>

#define VERSION "v0.8"

#define NODEID       2       // node ID used for this unit
#define NETWORKID  99
#define GATEWAYID  1
#define FREQUENCY  RF12_915MHZ //Match this with the version of your Moteino! (others: RF12_433MHZ, RF12_915MHZ)
#define KEY        "ABCDABCDABCDABCD"
#define ACK_TIME   50  // # of ms to wait for an ack
#define TRANSMITPERIOD 5000 //transmit a packet to gateway so often (in ms)

#define SERIAL_BAUD      115200

#define BUFFER_SIZE  128
#define CMD_BOOT     0x10
#define CMD_CONFIG   0x11
#define CMD_READING  0x21
#define CMD_ALERT    0x31
#define PRECISION    100

#define APIN_BATTERY    0  // data connection for battery
#define APIN_LDR        1  // data connection for photocell
#define DPIN_ONEWIRE    3  // data connection for the Dallas OneWire bus
#define DPIN_HALL       7  // data connection for hall effect
#define DPIN_SENSOR_LED 8
#define DPIN_RF_LED     9

#define DEFAULT_LED_DELAY 5

#define VDD_RATIO 1.622

struct SensorData {
  boolean available;
  boolean status;
  unsigned int ldr;
  unsigned int hall;
  float tempInsideC;
  float tempOutsideC;
  float batteryVoltage;
};

struct SensorThresholds {
  unsigned int light;
  unsigned int door;
  float minTempInsideC;
  float maxTempInsideC;
  float minTempOutsideC;
  float maxTempOutsideC;
};

// rfm12b radio
RFM12B radio;
char buffer[BUFFER_SIZE];
long lastPeriod = -1;

// dallas onewire configuration
OneWire oneWire(DPIN_ONEWIRE);
DallasTemperature oneWireSensors(&oneWire);
DeviceAddress thermometerInside  = { 0x28, 0xC3, 0x42, 0x77, 0x03, 0x00, 0x00, 0x78 };
DeviceAddress thermometerOutside  = { 0x10, 0x74, 0x6C, 0x53, 0x02, 0x08, 0x00, 0xE6 };

// container for sensor data
SensorData sensorData;
SensorThresholds sensorThresholds;


/******************************************************************************
 * Main Arduino Subroutines
 ******************************************************************************/

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  
  // open serial communications and wait for port to open:
  Serial.begin(SERIAL_BAUD);
  while (!Serial) ; // Needed for Leonardo only
  Serial.print("RF Freezer Monitor - ");
  Serial.println(VERSION);
  
  // radio initialization
  setup_radio();
  
  // sensor initialization
  setup_sensors();
  
  // define default sensor thresholds
  setup_sensor_thresholds();
  
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {

  // process inbound messages
  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      Serial.print('[');Serial.print(radio.GetSender(), DEC);Serial.print("] ");
      for (byte i = 0; i < *radio.DataLen; i++)
        Serial.print((char)radio.Data[i]);

      if (radio.ACKRequested())
      {
        radio.SendACK();
        Serial.print(" - ACK sent.");
      }
      blink_led(DPIN_RF_LED, DEFAULT_LED_DELAY);
    }
  }
  
  // generate outbound messages
  if ((int)(millis()/TRANSMITPERIOD) > lastPeriod) {
    lastPeriod++;
    
    if (!sensorData.status) {
      log();
      send_reading(CMD_ALERT);
      blink_led(DPIN_RF_LED, DEFAULT_LED_DELAY);
    }

  }
  
}

//------------------------------------------------------------------------------
void EmBencode::PushChar (char ch) {
  int len = strlen(buffer);
  buffer[len + 1] = buffer[len];
  buffer[len] = ch;
}

//------------------------------------------------------------------------------
void blink_led(byte pin, int delay_ms) {
  digitalWrite(pin, HIGH);
  delay(delay_ms);
  digitalWrite(pin, LOW);
}

//------------------------------------------------------------------------------
// Compares each sensor's value against its expectation, returning true if the
// values are acceptable or false if they fall outside the expectations.
int check_status() {
  int status = HIGH;
  if (sensorData.ldr > sensorThresholds.light) {
    status = LOW;
  }
  if (sensorData.hall == sensorThresholds.door) {
    status = LOW;
  }
  if (sensorData.tempInsideC < sensorThresholds.minTempInsideC) {
    status = LOW;
  }
  if (sensorData.tempInsideC > sensorThresholds.maxTempInsideC) {
    status = LOW;
  }
  return status;
}

//------------------------------------------------------------------------------
// Logs the current sensor values.
void log() {
  Serial.print("Status: "); 
  Serial.print(sensorData.status);
  Serial.print(", Light: "); 
  Serial.print(sensorData.ldr);
  Serial.print(", Hall: "); 
  Serial.print(sensorData.hall);
  Serial.print(", Temp: "); 
  Serial.print(sensorData.tempInsideC);
  Serial.print("C, "); 
  Serial.print(sensorData.tempOutsideC);
  Serial.print("C");
  Serial.print(", Battery: "); 
  Serial.println(sensorData.batteryVoltage);
}

//------------------------------------------------------------------------------
// Retrieves the new sensor values.
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
  
  // read battery voltage
  int val = analogRead(APIN_BATTERY);
  float pinVoltage = val * 0.00488;
  sensorData.batteryVoltage = pinVoltage * VDD_RATIO;

  // enable access to sensor data  
  sensorData.available = true;
 
  // update status led
  sensorData.status = check_status();
  digitalWrite(DPIN_SENSOR_LED, !sensorData.status); 
  
}

//------------------------------------------------------------------------------
void send_reading(unsigned int command) {
  
    // assemble sensor reading message
    int batteryVoltage = int(sensorData.batteryVoltage * 100);
    int tempInsideC = int(sensorData.tempInsideC * 100);
    int tempOutsideC = int(sensorData.tempOutsideC * 100);
    
    buffer[0] = '\0';
    EmBencode encoder;
    encoder.push(NODEID);
    encoder.push(command);
    encoder.startDict();
        encoder.push("status");
        encoder.push(sensorData.status);
        encoder.push("ldr");
        encoder.push(sensorData.ldr);
        encoder.push("hall");
        encoder.push(sensorData.hall);
        encoder.push("battery");
        encoder.push(batteryVoltage);
        encoder.push("tempInsideC");
        encoder.push(tempInsideC);
        encoder.push("tempOutsideC");
        encoder.push(tempOutsideC);
    encoder.endDict();
    Serial.print("reading [");
    Serial.print(buffer);
    Serial.print("]");
    
    // transmit reading to gateway
    radio.Send(GATEWAYID, buffer, strlen(buffer), true);
    Serial.print(" : waiting for ACK - ");
    if (waitForAck(GATEWAYID)) Serial.println("<OK>");
    else Serial.println("<NONE>");
}

//------------------------------------------------------------------------------
// Initializes the RFM12B radio.
void setup_radio() {
  radio.Initialize(NODEID, FREQUENCY, NETWORKID, 0);
  radio.Encrypt((byte*)KEY);
  sprintf(buffer, "Transmitting at %d Mhz...", FREQUENCY == RF12_433MHZ ? 433 : FREQUENCY== RF12_868MHZ ? 868 : 915);
  Serial.println(buffer);
}

//------------------------------------------------------------------------------
// Initializes the sensors.
void setup_sensors() {
  
  // initialize sensor connections
  pinMode(APIN_LDR, INPUT);
  pinMode(DPIN_HALL, INPUT);
  pinMode(DPIN_SENSOR_LED, OUTPUT);
  pinMode(DPIN_RF_LED, OUTPUT);
  oneWireSensors.begin();
  
  // setup background timer to interogate sensors  
  Timer1.initialize(1000000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt(interogate_sensors); // attach the service routine here
}

//------------------------------------------------------------------------------
void setup_sensor_thresholds() {
  buffer[0] = NODEID;
  buffer[1] = CMD_BOOT;
  radio.Wakeup();
  radio.Send(GATEWAYID, buffer, 10, true);
  if (waitForAck(GATEWAYID)) {
    Serial.println("RF Gateway Found - Loading sensor defaults...");
  } else {
    Serial.println("No RF Gateway - Using factory sensor defaults...");
  }
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

