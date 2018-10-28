#ifndef AQUARIUM_TYPES_H
#define AQUARIUM_TYPES_H

#include <cstdint>

/* Aquarium Sensor USB Protocol
 * 
 * [ id ][    payload     ]
 * 
 * 
 * 
 */
constexpr char AQ_MSG_ECHO = 0;

constexpr char AQ_MSG_LED = 1;
struct message_led {
    char id;
    char red_led;   /* 0 off, 1 on, 2 blink */
    char green_led;
};

constexpr char AQ_MSG_LIGHT = 2;
constexpr char AQ_MSG_TEMPERATURE = 3;
constexpr char AQ_MSG_TIME = 4;

constexpr char AQ_MSG_DIO = 5;
struct message_dio {
    char id;
    char channel;       /*  0 or 1 */
    uint16_t duty;      /*  16-bit pwm value */
    uint16_t duration;  /*  milliseconds to pulse output, 0 for constant */
};




#endif  /* AQUARIUM_TYPES_H */
