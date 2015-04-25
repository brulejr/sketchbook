/*
  Message.h - Library for structured messages.
  Created by Jon R. Brule, January 19, 2015.
  Released into the public domain.
*/
#ifndef Message_h
#define Message_h

#include "Arduino.h"

#define MSG_LENGTH        20
#define MSG_DATA_LENGTH   (MSG_LENGTH - 6)

#define RF_TO_MQTT     0x10
#define MQTT_TO_RF     0x20

#define MSG_ALERT      0x42
#define MSG_BOOTSTRAP  0x01
#define MSG_COMMAND    0x43
#define MSG_INFO       0x49
#define MSG_READING    0x52
#define MSG_WARNING    0x62

typedef struct {
    byte type;
    byte source;
    byte destination;
    byte component;
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
