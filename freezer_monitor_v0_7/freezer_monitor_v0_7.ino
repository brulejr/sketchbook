// Test sketch that is loaded on slave Moteinos
// It will send an encrypted message to the master/gateway every TRANSMITPERIOD
// It will respond to any ACKed messages from the master
// Every 3rd message will also be ACKed (request ACK from master).
#include <RFM12B.h>

#define VERSION "v0.7"

#define MYID         2       // node ID used for this unit
#define NETWORKID   99
#define GATEWAYID    1
#define FREQUENCY  RF12_915MHZ //Match this with the version of your Moteino! (others: RF12_433MHZ, RF12_915MHZ)
#define KEY  "ABCDABCDABCDABCD"
#define TRANSMITPERIOD 600 //transmit a packet to gateway so often (in ms)

#define SERIAL_BAUD      115200
#define ACK_TIME             50  // # of ms to wait for an ack

#define BUFFER_SIZE  50
#define CMD_BOOT     0x10
#define CMD_READING  0x11

int interPacketDelay = 1000; //wait this many ms between sending packets
char input = 0;
RFM12B radio;

boolean requestACK = false;
byte sendSize=0;
char payload[] = "123 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char buffer[BUFFER_SIZE];

long lastPeriod = -1;


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
  radio.Initialize(MYID, FREQUENCY, NETWORKID, 0);
  radio.Encrypt((byte*)KEY);
  sprintf(buffer, "Transmitting at %d Mhz...", FREQUENCY == RF12_433MHZ ? 433 : FREQUENCY== RF12_868MHZ ? 868 : 915);
  Serial.println(buffer);
  
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
      Blink(9,5);
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
    Blink(9,5);
  }
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

//------------------------------------------------------------------------------
void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
