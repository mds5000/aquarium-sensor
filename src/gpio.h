#ifndef GPIO_H
#define GPIO_H

#include "sam.h"
#include "SEGGER_RTT.h"

constexpr uint8_t GROUP_A = 0;
constexpr uint8_t GROUP_B = 1;

class Gpio {
public:
    constexpr static bool Input = 0;
    constexpr static bool Output = 1;

    Gpio(uint8_t group_, uint8_t pin_) : group(group_), pin(pin_) {
            PORT->Group[group].PINCFG[pin].bit.INEN = 1;
    }

    void direction(bool out) {
        if (out)
            PORT->Group[group].DIRSET.reg = 1 << pin;
        else
            PORT->Group[group].DIRCLR.reg = 1 << pin;
    }

    void output(bool high) {
        if (high)
            PORT->Group[group].OUTSET.reg = 1 << pin;
        else
            PORT->Group[group].OUTCLR.reg = 1 << pin;
    }

    bool get() {
        return PORT->Group[group].IN.reg & (1 << pin);
    }

    void write(bool high) {
        direction(true);
        output(high);
    }
    bool read() {
        direction(false);
        return get();
    }

private:
    uint8_t group;
    uint8_t pin;
};

#endif /* GPIO_H */