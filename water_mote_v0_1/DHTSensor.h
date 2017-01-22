/*
  DHTSensor.h - Encapsulates temperature / humidity measurement.
  Created by Jon R. Brule, January 22, 2017.
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
