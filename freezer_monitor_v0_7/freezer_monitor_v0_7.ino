/* -----------------------------------------------------------------------------
   Freezer Monitor
  
   Accepts item numbers from PS/2 connection and submits initiates an Item
   process via a RESTful web service.
 
   Circuit:
   * RFM12B attached to digital pins 10, 11, 12, 13
   * LDR sensor attached to analog pin 0
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

#define VERSION "v0.7"

#define MYID       2       // node ID used for this unit
#define NETWORKID  99
#define GATEWAYID  1
#define FREQUENCY  RF12_915MHZ //Match this with the version of your Moteino! (others: RF12_433MHZ, RF12_915MHZ)
#define KEY        "ABCDABCDABCDABCD"
#define ACK_TIME   50  // # of ms to wait for an ack
#define TRANSMITPERIOD 600 //transmit a packet to gateway so often (in ms)

#define SERIAL_BAUD      115200

#define BUFFER_SIZE  50
#define CMD_BOOT     0x10
#define CMD_READING  0x11

#define APIN_LDR        0  // data connection for photocell
#define DPIN_ONEWIRE    3  // data connection for the Dallas OneWire bus
#define DPIN_HALL       7  // data connection for hall effect
#define DPIN_SENSOR_LED 8
#define DPIN_RF_LED     9

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

// rfm12b radio
int interPacketDelay = 1000; //wait this many ms between sending packets
char input = 0;
RFM12B radio;

boolean requestACK = false;
byte sendSize=0;
char payload[] = "123 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char buffer[BUFFER_SIZE];

long lastPeriod = -1;

// dallas onewire configuration
OneWire oneWire(DPIN_ONEWIRE);
DallasTemperature oneWireSensors(&oneWire);
DeviceAddress thermometerInside  = { 0x10, 0x9B, 0x96, 0x53, 0x02, 0x08, 0x00, 0xF0 };
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

  //check for any received packets
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
      blink_led(DPIN_RF_LED, 5);
    }
  }
  
  if ((int)(millis()/TRANSMITPERIOD) > lastPeriod)
  {
    lastPeriod++;
    //Send data periodically to GATEWAY
    Serial.print("Sending[");
    Serial.print(sendSize);
    Serial.print("]: ");
    for(byte i = 0; i < sendSize; i++)
      Serial.print((char)payload[i]);
    
    requestACK = ((sendSize % 3) == 0); //request ACK every 3rd xmission
    radio.Send(GATEWAYID, payload, sendSize, requestACK);
    if (requestACK)
    {
      Serial.print(" - waiting for ACK...");
      if (waitForAck(GATEWAYID)) Serial.print("ok!");
      else Serial.print("nothing...");
    }
    
    sendSize = (sendSize + 1) % 31;
    Serial.println();
    blink_led(DPIN_RF_LED, 5);
    log();
  }
  
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
// Logs the current sensor values.
void log() {
  Serial.print("Light: "); 
  Serial.print(sensorData.ldr);
  Serial.print("\t Hall: "); 
  Serial.print(sensorData.hall);
  Serial.print("\t Temp: "); 
  Serial.print(sensorData.tempInsideC);
  Serial.print("C, "); 
  Serial.print(sensorData.tempOutsideC);
  Serial.println("C");
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

  // enable access to sensor data  
  sensorData.available = true;
 
  // update status led
  digitalWrite(DPIN_SENSOR_LED, check_status()); 
  
}

//------------------------------------------------------------------------------
// Initializes the RFM12B radio.
void setup_radio() {
  radio.Initialize(MYID, FREQUENCY, NETWORKID, 0);
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
  buffer[0] = MYID;
  buffer[1] = CMD_BOOT;
  radio.Wakeup();
  radio.Send(GATEWAYID, payload, 10, true);
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

