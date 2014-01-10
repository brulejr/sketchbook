/* -----------------------------------------------------------------------------
   Freezer Monitor
  
   Accepts item numbers from PS/2 connection and submits initiates an Item
   process via a RESTful web service.
 
   Circuit:
   * Ethernet shield attached to pins 10, 11, 12, 13
   * PS/2 keyboard attached to pins 3 (clock), 4 (data)
   * Status LEDs attached to pins 7 (red), 8 (yellow), 9 (green)
 
   Created 06-JAN-2014 by Jon Brule
----------------------------------------------------------------------------- */

#include <SPI.h>
#include <Ethernet.h>
#include <RestServer.h>

#include <OneWire.h>
#include <DallasTemperature.h> 

#include <TimerOne.h>


#define REST_SERVER_PORT 5555
#define VERSION "v0.5"
#define TIMEZONE "-0500"

#define DPIN_ONEWIRE 3  // data connection for the Dallas OneWire bus
#define DPIN_HALL    8  // data connection for hall effect
#define APIN_LDR     0  // data connection for photocell

#define LED_STATUS 9

#define LOOP_DELAY 0

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

// Controller MAC address - must be unique on network for DHCP to work
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,100,132);
IPAddress gateway(192,168,100,1);
IPAddress subnet(255,255,255,0);

// RESTful Server
RestServer restServer(REST_SERVER_PORT);

// Dallas OneWire configuration
OneWire oneWire(DPIN_ONEWIRE);
DallasTemperature oneWireSensors(&oneWire);
DeviceAddress thermometerInside  = { 0x10, 0x9B, 0x96, 0x53, 0x02, 0x08, 0x00, 0xF0 };
DeviceAddress thermometerOutside  = { 0x10, 0x74, 0x6C, 0x53, 0x02, 0x08, 0x00, 0xE6 };


// container for sensor data
SensorData sensorData;
SensorThresholds sensorThresholds;

char content[128];
char buffer[64];


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
  Serial.print("Networked Environment Sensor - ");
  Serial.println(VERSION);
  
  // sensor configurations
  pinMode(APIN_LDR, INPUT);
  pinMode(DPIN_HALL, INPUT);

  // output configuration
  pinMode(LED_STATUS, OUTPUT);
  
  // start up the one-wire sensors
  oneWireSensors.begin();
  
  // define default sensor thresholds
  default_sensor_thresholds();
  
  // obtain dhcp address
  Serial.println("Attempting to obtain a DHCP lease...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP - using fallback");
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  
  // setup RESTful processor
  restServer.attach("", handle_healthcheck);
  restServer.attach("sensors", handle_sensors);
  restServer.attach("thresholds", handle_thresholds);
  restServer.begin();
  Serial.print("server started on ");
  Serial.println(Ethernet.localIP());
  
  Timer1.initialize(1000000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt(interogate_sensors); // attach the service routine here
  
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  restServer.process();
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
void default_sensor_thresholds() {
  sensorThresholds.light = 50;
  sensorThresholds.door = HIGH;
  sensorThresholds.minTempInsideC = 0.00;
  sensorThresholds.maxTempInsideC = 2.00;
  sensorThresholds.minTempOutsideC = 19.00;
  sensorThresholds.maxTempOutsideC = 20.00;
}

//------------------------------------------------------------------------------
void handle_healthcheck(RestRequest *request, EthernetClient *client) {
  reset_buffers();
  strcat(content, "{");
  sprintf_string_field(buffer, true, "health", "OK");
  strcat(content, buffer);  
  sprintf_string_field(buffer, false, "version", VERSION);
  strcat(content, buffer);  
  strcat(content, "}");
  restServer.generate_response(client, content);
}

//------------------------------------------------------------------------------
void handle_sensors(RestRequest *request, EthernetClient *client) {
  while (!sensorData.available) {};
  reset_buffers();
  strcat(content, "{");
  sprintf_int_field(buffer, true, "status", check_status());
  strcat(content, buffer);  
  sprintf_int_field(buffer, false, "light", sensorData.ldr);
  strcat(content, buffer);  
  sprintf_int_field(buffer, false, "door", sensorData.hall);
  strcat(content, buffer);  
  sprintf_float_field(buffer, false, "tempInsideC", sensorData.tempInsideC, 100);
  strcat(content, buffer);  
  sprintf_float_field(buffer, false, "tempOutsideC", sensorData.tempOutsideC, 100);
  strcat(content, buffer);  
  strcat(content, "}");
  restServer.generate_response(client, content);
}

//------------------------------------------------------------------------------
void handle_thresholds(RestRequest *request, EthernetClient *client) {
  reset_buffers();
  strcat(content, "{");
  sprintf_int_field(buffer, true, "light", sensorThresholds.light);
  strcat(content, buffer);  
  sprintf_int_field(buffer, false, "door", sensorThresholds.door);
  strcat(content, buffer);  
  sprintf_float_field(buffer, false, "minTempInsideC", sensorThresholds.minTempInsideC, 100);
  strcat(content, buffer);  
  sprintf_float_field(buffer, false, "maxTempInsideC", sensorThresholds.maxTempInsideC, 100);
  strcat(content, buffer);  
  sprintf_float_field(buffer, false, "minTempOutsideC", sensorThresholds.minTempOutsideC, 100);
  strcat(content, buffer);  
  sprintf_float_field(buffer, false, "maxTempOutsideC", sensorThresholds.maxTempOutsideC, 100);
  strcat(content, buffer);  
  strcat(content, "}");
  restServer.generate_response(client, content);
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
void reset_buffers() {
  content[0] = '\0';
  buffer[0] = '\0';
}

//------------------------------------------------------------------------------
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)
void sprintf_float_field(char *buffer, boolean firstField, char *field, float value, unsigned int precision){
    int dec = int(value);
    unsigned int frac;
    if(value >= 0) {
        frac = (value - int(value)) * precision;
    } else {
        frac = (int(value) - value) * precision;
    }
    sprintf(buffer, "%s\"%s\": %d.%d", (firstField) ? "" : ", ", field, dec, frac);
} 

//------------------------------------------------------------------------------
void sprintf_int_field(char *buffer, boolean firstField, char *field, int value){
    sprintf(buffer, "%s\"%s\": %d", (firstField) ? "" : ", ", field, value);
}

//------------------------------------------------------------------------------
void sprintf_string_field(char *buffer, boolean firstField, char *field, char *value){
    sprintf(buffer, "%s\"%s\": \"%s\"", (firstField) ? "" : ", ", field, value);
}

//------------------------------------------------------------------------------
void update_status() {
  digitalWrite(LED_STATUS, check_status());
}
