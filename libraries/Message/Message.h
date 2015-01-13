/*
  Message.h - Library for structured messages.
  Created by Jon R. Brule, January 4, 2015.
  Released into the public domain.
*/
#ifndef Message_h
#define Message_h

#include "Arduino.h"

#define MSG_LENGTH        20
#define MSG_DATA_LENGTH   (MSG_LENGTH - 6)

#define RF_TO_MQTT    0x10
#define MQTT_TO_RF    0x20

#define MSG_ALERT     0x41
#define MSG_COMMAND   0x43
#define MSG_INFO      0x49
#define MSG_READING   0x52

typedef struct {
    byte direction;
    byte type;
    byte source;
    byte destination;
    int rssi;
    byte data[MSG_DATA_LENGTH];
} MessageRecord;

typedef struct {
    union {
        byte raw[MSG_LENGTH];
        MessageRecord msg;
    };
} Message;

#endif
