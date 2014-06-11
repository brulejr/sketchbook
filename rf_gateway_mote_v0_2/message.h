#define MSG_LENGTH        16
#define MSG_BODY_LENGTH   MSG_LENGTH - 3

typedef struct {
    byte msgtype;
    byte network;
    byte node;
    byte data[MSG_BODY_LENGTH];
} Message;

typedef struct {
    union {
        byte raw[MSG_LENGTH];
        Message msg;
    };
} MessageData;

