/*
  Freezer.h - Library for freezer sensor measurement.
  Created by Jon R. Brule, June 8, 2014.
  Released into the public domain.
*/
#ifndef Freezer_h
#define Freezer_h

#include "Arduino.h"
#include <RFM69.h>

#define SERIAL        1

#define APIN_BATTERY  0  // data connection for battery
#define APIN_LDR      1  // data connection for photocell
#define DPIN_HALL     7  // data connection for hall effect
#define VOLTAGE       3.3

#define SMOOTHING     3

#define RAW_LENGTH    16

#define NODEID        2   // unique for each node on same network
#define GATEWAYID     1
#define NETWORKID     99  // same for all nodes that talk to each other
#define FREQUENCY     RF69_915MHZ

#define MSG_COMMAND   0x43
#define MSG_INFO      0x49
#define MSG_READING   0x52

struct SensorData {
    byte msgtype;
    byte network;
    byte node;
    struct {
        byte light;   // light sensor: 0..255
        byte door;    // door sensor: 0..1
        int battery;  // battery voltage
    };
};

struct MessageData {
    union {
        byte raw[RAW_LENGTH];
        SensorData sensor;
    };
};

class Freezer
{
  public:
    Freezer(byte reportCycle);
    void measure();
    boolean isReportReady();
    MessageData* report();
  protected:
    void doMeasure();
    void doReport();
    int smoothedAverage(int prev, int next);
  private:
    byte _reportCycle;
    byte _smooth;
    byte _reportCnt;
    boolean _firstTime;
    boolean _reportReady;
    SensorData _reading;
    MessageData _message;
};

#endif
