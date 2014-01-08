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

#include <Time.h>
#include <Wire.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#include <OneWire.h>
#include <DallasTemperature.h> 


#define REST_SERVER_PORT 5555
#define VERSION "v0.4"
#define TIMEZONE "-0500"

#define DPIN_ONEWIRE 3  // data connection for the Dallas OneWire bus
#define DPIN_HALL    8  // data connection for hall effect
#define APIN_LDR     0  // data connection for photocell

#define LED_STATUS 9

#define LOOP_DELAY 0

struct SensorData {
  time_t timestamp;
  unsigned int ldr;
  unsigned int hall;
  float tempInsideC;
};

struct SensorThresholds {
  unsigned int light;
  unsigned int door;
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
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
  Serial.print("Networked Environment Sensor - ");
  Serial.println(VERSION);
  
  // synchronize clock with RTC
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");
  
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
}

//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  interogate_sensors();
  update_status();
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
  return status;
}

//------------------------------------------------------------------------------
void default_sensor_thresholds() {
  sensorThresholds.light = 50;
  sensorThresholds.door = HIGH;
}

//------------------------------------------------------------------------------
void handle_healthcheck(RestRequest *request, EthernetClient *client) {
  char content[32] = { '\0' };
  char buffer[128] = { '\0' };
  strcat(content, "{");
  strcat(content, "\"health\": \"OK\"");
  strcat(content, "}");
  restServer.generate_response(client, content);
}

//------------------------------------------------------------------------------
void handle_sensors(RestRequest *request, EthernetClient *client) {
  char content[128] = { '\0' };
  char buffer[128] = { '\0' };
  strcat(content, "{");
  sprintf_int_field(buffer, true, "status", check_status());
  strcat(content, buffer);  
  sprintf_timestamp_field(buffer, true, "timestamp", sensorData.timestamp);
  strcat(content, buffer);
  sprintf_int_field(buffer, false, "light", sensorData.ldr);
  strcat(content, buffer);  
  sprintf_int_field(buffer, false, "door", sensorData.hall);
  strcat(content, buffer);  
  sprintf_float_field(buffer, false, "tempInsideC", sensorData.tempInsideC, 100);
  strcat(content, buffer);  
  strcat(content, "}");
  restServer.generate_response(client, content);
}

//------------------------------------------------------------------------------
void handle_thresholds(RestRequest *request, EthernetClient *client) {
  char content[32] = { '\0' };
  char buffer[16] = { '\0' };
  strcat(content, "{");
  sprintf_int_field(buffer, true, "light", sensorThresholds.light);
  strcat(content, buffer);  
  sprintf_int_field(buffer, false, "door", sensorThresholds.door);
  strcat(content, buffer);  
  strcat(content, "}");
  restServer.generate_response(client, content);
}

//------------------------------------------------------------------------------
void interogate_sensors() {
  
  // read real-time clock
  sensorData.timestamp = now();
  
  // read light level
  sensorData.ldr = map(analogRead(APIN_LDR), 0, 1023, 0, 255);

  // read door status
  sensorData.hall = digitalRead(DPIN_HALL);
  
  // read one-wire sensors
  oneWireSensors.requestTemperatures();
  sensorData.tempInsideC = oneWireSensors.getTempC(thermometerInside);
  
}

//------------------------------------------------------------------------------
void sprintf_timestamp_field(char *buffer, boolean firstField, char *field, time_t timestamp) {
  sprintf(buffer, "%s\"%s\": \"%4d-%02d-%02dT%02d:%02d:%02d%s\"",
    (firstField) ? ", " : "",
    field, 
    year(timestamp), 
    month(timestamp), 
    day(timestamp),
    hour(timestamp),
    minute(timestamp),
    second(timestamp),
    TIMEZONE);
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
void update_status() {
  digitalWrite(LED_STATUS, check_status());
}
