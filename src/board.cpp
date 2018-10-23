#include "board.h"

Gpio dio_b = Gpio(GROUP_A, 0);
Gpio dio_a = Gpio(GROUP_A, 1);

Gpio gpio_temp = Gpio(GROUP_A, 9);

Gpio led_red = Gpio(GROUP_A, 14);
Gpio led_green = Gpio(GROUP_A, 15);

Gpio usb_vsense = Gpio(GROUP_A, 10);

__attribute__((section(".ramfunc"))) void delay_us(uint32_t us)
{
  __asm(
  "mydelay: \n"
  " sub  r0, r0, #1 \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " nop             \n"  // 1 cycle
  " bne  mydelay    \n"  // 2 if taken, 1 otherwise
  );                       
}

__attribute__((section(".ramfunc"))) void delay_ms(uint32_t ms) {
  for(ms; ms > 0; ms--)
    delay_us(1000);
}