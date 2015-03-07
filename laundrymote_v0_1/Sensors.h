/*
  Sensors.h - Library for sensor measurement.
  Created by Jon R. Brule, March 5, 2015.
  Released into the public domain.
*/
#ifndef Sensors_h
#define Sensors_h

#include "Arduino.h"
#include "Reading.h"

typedef struct SensorData {
    byte battery;      // battery voltage
    byte light;        // 0 - 255
    byte temperature;  // temperature sensor: C * 10
    byte waterLeak;   // 0 or 1
};

class Sensors {
    public:
        Sensors();
        void measure();
        void report(SensorData* sensorData);
        static int readBattery(byte pin);
        static int readLight(byte pin);
        static int readTemperature(byte pin);
        static int readWaterLeak(byte pin);
    private:
        Reading* _battery;
        Reading* _light;
        Reading* _temperature;
        Reading* _waterLeak;
};

#endif
