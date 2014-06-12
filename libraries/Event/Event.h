/*
  Event.h - Library for event messages.
  Created by Jon R. Brule, June 10, 2014.
  Released into the public domain.
*/
#ifndef Event_h
#define Event_h

#include "Arduino.h"

#define EVENT_LENGTH        20
#define EVENT_DATA_LENGTH   EVENT_LENGTH - 4

#define EVENT_ALERT     0x41
#define EVENT_COMMAND   0x43
#define EVENT_INFO      0x49
#define EVENT_READING   0x52

typedef struct {
    byte type;
    byte network;
    byte source;
    byte destination;
    byte data[EVENT_DATA_LENGTH];
} EventRecord;

typedef struct {
    union {
        byte raw[EVENT_LENGTH];
        EventRecord event;
    };
} EventMessage;

#endif
