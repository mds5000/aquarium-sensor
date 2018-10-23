#ifndef BOARD_H
#define BOARD_H

#include <cstdint>

#include "gpio.h"

extern "C" {
    #include "SEGGER_RTT.h"
}


void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

extern Gpio dio_b;
extern Gpio dio_a;

extern Gpio gpio_temp;

extern Gpio led_red;
extern Gpio led_green;

extern Gpio usb_vsense;


#endif /* BOARD_H */