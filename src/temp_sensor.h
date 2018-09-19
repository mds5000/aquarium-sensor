#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdint.h>

#include "gpio.h"

#define COMMAND_SKIP_ROM         0xCC
#define COMMAND_START_CONVERSION 0x44
#define COMMAND_READ_SP          0xBE


class OneWire {
public:
    OneWire(Gpio& pin_) : pin(pin_) {}

    bool reset();

    void write_bit(bool bit);
    void write_byte(uint8_t byte);

    bool read_bit();
    uint8_t read_byte();

protected:
    Gpio& pin;
};


class Ds18b20 : public OneWire {
public:
    Ds18b20(Gpio& pin_) : OneWire(pin_) {}

    bool start_conversion();
    uint16_t get_temperature();

private:
    bool read_scratch_pad();
    uint8_t crc8(uint8_t* data, uint8_t len);

    uint8_t scratch_pad[9];
};

#endif /* TEMP_SENSOR_H */