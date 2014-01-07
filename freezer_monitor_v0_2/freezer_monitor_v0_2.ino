/* -----------------------------------------------------------------------------
   Freezer Monitor
  
   Accepts item numbers from PS/2 connection and submits initiates an Item
   process via a RESTful web service.
 
   Circuit:
   * Ethernet shield attached to pins 10, 11, 12, 13
   * PS/2 keyboard attached to pins 3 (clock), 4 (data)
   * Status LEDs attached to pins 7 (red), 8 (yellow), 9 (green)
 
   Created 04-JAN-2014 by Jon Brule
----------------------------------------------------------------------------- */

#include <SPI.h>
#include <Ethernet.h>
#include <RestServer.h>
#include <DHT.h>

#define REST_SERVER_PORT 5555

#define PIN_DHT1  3  // data connection for DHT11 (internal)
#define PIN_DHT2  4  // data connection for DHT11 (external)
#define PIN_HALL  8  // data connection for hall effect
#define PIN_LDR   0  // data connection for photocell

#define LED_STATUS 9

struct SensorData {
  int ldr;
  int hall;
  float humid1;
  float temp1;
  float humid2;
  float temp2;
};

// Controller MAC address - must be unique on network for DHCP to work
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,100,132);
IPAddress gateway(192,168,100,1);
IPAddress subnet(255,255,255,0);

// RESTful Server
RestServer restServer(REST_SERVER_PORT);

// DHT11 wiring
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
DHT dht1(PIN_DHT1, DHT11);
DHT dht2(PIN_DHT2, DHT11);

// container for sensor data
SensorData sensorData;


/******************************************************************************
 * Main Arduino Subroutines
 ******************************************************************************/

//------------------------------------------------------------------------------
// setup the arduino board
//
void setup() {
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Networked Environment Sensor");
  
  // sensor configurations
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_HALL, INPUT);
  dht1.begin();
  dht2.begin(); 

  // output configuration
  pinMode(LED_STATUS, OUTPUT);
  
  // obtain dhcp address
  Serial.println("Attempting to obtain a DHCP lease...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP - using fallback");
    Ethernet.begin(mac, ip, gateway, subnet);
  }
  
  // setup RESTful processor
  restServer.attach("", handle_healthcheck);
  restServer.attach("sensors", handle_sensors);
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
  
}


/******************************************************************************
 * Internal Subroutines
 ******************************************************************************/

//------------------------------------------------------------------------------
int check_status() {
  int status = LOW;
  if (sensorData.ldr > 50) {
    status = HIGH;
  }
  if (sensorData.hall == HIGH) {
    status = HIGH;
  }
  return status;
}

//------------------------------------------------------------------------------
void handle_healthcheck(RestRequest *request, EthernetClient *client) {
  char content[32] = { '\0' };
  strcat(content, "{ \"status\": \"OK\" }");
  restServer.generate_response(client, content);
}

//------------------------------------------------------------------------------
void handle_sensors(RestRequest *request, EthernetClient *client) {
  char content[64] = { '\0' };
  char buffer[64];
  strcat(content, "{");
  sprintIntField(buffer, true, "ldr", sensorData.ldr);
  strcat(content, buffer);  
  sprintIntField(buffer, false, "hall", sensorData.hall);
  strcat(content, buffer);  
  sprintFloatField(buffer, false, "humid1", sensorData.humid1, 100);
  strcat(content, buffer);  
  sprintFloatField(buffer, false, "temp1", sensorData.temp1, 100);
  strcat(content, buffer);  
  sprintFloatField(buffer, false, "humid2", sensorData.humid2, 100);
  strcat(content, buffer);  
  sprintFloatField(buffer, false, "temp2", sensorData.temp2, 100);
  strcat(content, buffer);  
  strcat(content, "}");
  restServer.generate_response(client, content);
}

//------------------------------------------------------------------------------
void interogate_sensors() {
  
   // read light level
  sensorData.ldr = analogRead(PIN_LDR);

  // read door status
  sensorData.hall = digitalRead(PIN_HALL);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  sensorData.humid1 = dht1.readHumidity();
  sensorData.temp1  = dht1.readTemperature();
  sensorData.humid2 = dht2.readHumidity();
  sensorData.temp2  = dht2.readTemperature();
  
}

//------------------------------------------------------------------------------
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)
void sprintFloatField(char *buffer, boolean firstField, char *field, float value, unsigned int precision){
    int dec = int(value);
    unsigned int frac;
    if(value >= 0) {
        frac = (value - int(value)) * precision;
    } else {
        frac = (int(value) - value) * precision;
    }
    if (firstField) {
      sprintf(buffer, "\"%s\": %d.%d", field, dec, frac);
    } else {
      sprintf(buffer, ", \"%s\": %d.%d", field, dec, frac);
    }
} 

//------------------------------------------------------------------------------
void sprintIntField(char *buffer, boolean firstField, char *field, int value){
    if (firstField) {
      sprintf(buffer, "\"%s\": %d", field, value);
    } else {
      sprintf(buffer, ", \"%s\": %d", field, value);
    }
} 

//------------------------------------------------------------------------------
void update_status() {
  digitalWrite(LED_STATUS, check_status());
}
