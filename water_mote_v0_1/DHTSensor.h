/*
  DHTSensor.h - Encapsulates temperature / humidity measurement.
  Created 22-JAN-2017 by Jon Brule
*/
#ifndef DHTSensor_h
#define DHTSensor_h

#include "Arduino.h"
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT11
#define MISSING_READING -3.4028235E+38

class DHTSensor
{
  public:
    DHTSensor(byte dataPin);
    float humidity();
    void setup();
    float temperature();
  private:
    DHT_Unified* _dht;
    byte _dataPin;
    bool _ready;
    sensor_t _sensor;
};

#endif
