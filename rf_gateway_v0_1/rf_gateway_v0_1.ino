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

#define BUFFER_SIZE  64

char buffer[BUFFER_SIZE];

uint8_t input[RF12_MAXDATA];
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
  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      digitalWrite(9,1);
      Serial.print('[');Serial.print(radio.GetSender(), DEC);Serial.print("] ");
      for (byte i = 0; i < *radio.DataLen; i++)
        Serial.print((char)radio.Data[i]);

      if (radio.ACKRequested())
      {
        byte theNodeID = radio.GetSender();
        radio.SendACK();
        //when a node requests an ACK, respond to the ACK and also send a packet requesting an ACK
        //This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
        Serial.print(" - ACK sent. Sending packet to node ");
        Serial.print(theNodeID);
        delay(10);
        radio.Send(theNodeID, "ACK TEST", 8, true);
        Serial.print(" - waiting for ACK...");
        if (waitForAck(theNodeID)) Serial.print("ok!");
        else Serial.print("nothing...");
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
// Initializes the RFM12B radio.
void setup_radio() {
  radio.Initialize(NODEID, FREQUENCY, NETWORKID, 0);
  radio.Encrypt((byte*)KEY);
  sprintf(buffer, "Transmitting at %d Mhz...", FREQUENCY == RF12_433MHZ ? 433 : FREQUENCY== RF12_868MHZ ? 868 : 915);
  Serial.println(buffer);
}
