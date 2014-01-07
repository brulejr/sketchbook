/*
  Web Server
 
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

#define PIN_DHT1  3  // data connection for DHT11 (internal)
#define PIN_DHT2  4  // data connection for DHT11 (external)
#define PIN_HALL  8  // data connection for hall effect
#define PIN_LDR   0  // data connection for photocell

#define LED_STATUS 9


// DHT11 wiring
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht1(PIN_DHT1, DHT11);
DHT dht2(PIN_DHT2, DHT11);


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,100,132);


// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);


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

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  
  // read light level
  int ldr = analogRead(PIN_LDR);

  // read door status
  int hall = digitalRead(PIN_HALL);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humid1 = dht1.readHumidity();
  float temp1  = dht1.readTemperature();
  float humid2 = dht2.readHumidity();
  float temp2  = dht2.readTemperature();

  digitalWrite(LED_STATUS, checkStatus(ldr, hall, humid1, temp1, humid2, temp2));
  
  log(ldr, hall, humid1, temp1, humid2, temp2);
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");       
          }
          
          
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(1);
    
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}


int checkStatus(int ldr, int hall, float humid1, float temp1, float humid2, float temp2) {
  
  int status = LOW;
  if (ldr > 50) {
    status = HIGH;
  }
  if (hall == HIGH) {
    status = HIGH;
  }
  return status;
}


void log(int ldr, int hall, float humid1, float temp1, float humid2, float temp2) {
  Serial.print("Light: "); 
  Serial.print(ldr);
  Serial.print("\t Hall: "); 
  Serial.print(hall);
  Serial.print("\t Humid: "); 
  Serial.print(humid1);
  Serial.print(","); 
  Serial.print(humid2);
  Serial.print("%");
  Serial.print("\t Temp: "); 
  Serial.print(temp1);
  Serial.print(","); 
  Serial.print(temp2);
  Serial.println("*C");
}
