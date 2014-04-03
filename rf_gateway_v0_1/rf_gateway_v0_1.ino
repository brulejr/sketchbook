// Test sketch that is loaded on master Moteinos
// It will listen for any packets received from slave Moteinos
// If an ACK request message is received, it sends and ACK and also
// sends an ACKed message to the slave, ensuring that 2-way
// communication is functional on the slave Moteino

#include <RFM12B.h>
#include <SPI.h>
//#include <Ethernet.h>


#define VERSION "v0.7"

#define NODEID      1       // node ID used for this unit
#define NETWORKID  99
#define FREQUENCY  RF12_915MHZ //Match this with the version of your Moteino! (others: RF12_433MHZ, RF12_915MHZ)
#define KEY        "ABCDABCDABCDABCD"
#define ACK_TIME   50  // # of ms to wait for an ack

#define SERIAL_BAUD 115200

#define BUFFER_SIZE 128

//byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };

char buffer[BUFFER_SIZE];

RFM12B radio;
//EthernetClient client;


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
    
  // ethernet initialization
  //setup_ethernet();
  
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
      Serial.print('[');Serial.print(radio.GetSender(), DEC);Serial.print("] ");
      for (byte i = 0; i < *radio.DataLen; i++)
        Serial.print((char)radio.Data[i]);

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

//------------------------------------------------------------------------------
// Initializes the Ethernet.
/*void setup_ethernet() {
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;);
  }
  Serial.print("Ethernet address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
}*/

//------------------------------------------------------------------------------
// Initializes the RFM12B radio.
void setup_radio() {
  radio.Initialize(NODEID, FREQUENCY, NETWORKID, 0);
  radio.Encrypt((byte*)KEY);
  sprintf(buffer, "Transmitting at %d Mhz...", FREQUENCY == RF12_433MHZ ? 433 : FREQUENCY== RF12_868MHZ ? 868 : 915);
  Serial.println(buffer);
}
