/*
  Sensor.h - Sensor definition
  Created by Jon R. Brule, April 15, 2015.
  Released into the public domain.
*/
#ifndef Sensor_h
#define Sensor_h

class Sensor {
  public:
    inline virtual void begin(){ /*nothing*/ };
    virtual int read() = 0;
};

#endif
