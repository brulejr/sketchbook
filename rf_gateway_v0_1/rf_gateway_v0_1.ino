// Test sketch that is loaded on master Moteinos
// It will listen for any packets received from slave Moteinos
// If an ACK request message is received, it sends and ACK and also
// sends an ACKed message to the slave, ensuring that 2-way
// communication is functional on the slave Moteino

#include <RFM12B.h>


#define VERSION "v0.7"

#define NODEID      1       // node ID used for this unit
#define NETWORKID  99
#define FREQUENCY  RF12_915MHZ //Match this with the version of your Moteino! (others: RF12_433MHZ, RF12_915MHZ)
#define KEY        "ABCDABCDABCDABCD"
#define ACK_TIME   50  // # of ms to wait for an ack

#define SERIAL_BAUD 115200

#define BUFFER_SIZE 128

char buffer[BUFFER_SIZE];

RFM12B radio;

byte recvCount = 0;


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
  Serial.print("RF Gateway - ");
  Serial.println(VERSION);
    
  // radio initialization
  setup_radio();
  
  // setup flashy pin
  pinMode(9, OUTPUT);

}


//------------------------------------------------------------------------------
// main processing loop
//
void loop() {
  
  // process inbound messages
  if (radio.ReceiveComplete()) {
    if (radio.CRCPass()) {
      digitalWrite(9,1);
      assemble_message();

      if (radio.ACKRequested()) {
        byte theNodeID = radio.GetSender();
        radio.SendACK();
      }
      delay(5);
      digitalWrite(9,0);
    }
    else Serial.print("BAD-CRC");
    
    Serial.println();
  }
}

// wait a few milliseconds for proper ACK to me, return true if indeed received
static bool waitForAck(byte theNodeID) {
  long now = millis();
  while (millis() - now <= ACK_TIME) {
    if (radio.ACKReceived(theNodeID))
      return true;
  }
  return false;
}

void assemble_message() {
  
  String raw = String((char*) radio.Data);
  raw = raw.substring(0, *radio.DataLen);
  int sep1 = raw.indexOf(":");
  int sep2 = raw.indexOf(":", sep1 + 1);
  
  String content = "{";
  content += "\"node\":" + raw.substring(0, sep1);
  content += ",\"cmd\":\"" + raw.substring(sep1 + 1, sep2) + "\"";
  content += ",\"data\":\"" + raw.substring(sep2 + 1, raw.length()) + "\"";
  content += "}";
  Serial.print("raw<");
  Serial.print(raw.length());
  Serial.print("> - ");
  Serial.println(raw);
  Serial.print("content<");
  Serial.print(content.length());
  Serial.print("> - ");
  Serial.println(content);
  
  content.toCharArray(buffer, BUFFER_SIZE);
}

//------------------------------------------------------------------------------
// Initializes the RFM12B radio.
void setup_radio() {
  radio.Initialize(NODEID, FREQUENCY, NETWORKID, 0);
  radio.Encrypt((byte*)KEY);
  sprintf(buffer, "Transmitting at %d Mhz...", FREQUENCY == RF12_433MHZ ? 433 : FREQUENCY== RF12_868MHZ ? 868 : 915);
  Serial.println(buffer);
}
