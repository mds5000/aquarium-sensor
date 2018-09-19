#include "temp_sensor.h"

#include "board.h"

bool OneWire::reset() {
    pin.write(1);
    pin.output(0);
    delay_us(600);

    pin.direction(Gpio::Input);
    delay_us(60);
    bool resp = pin.get();

    delay_us(500);
    pin.write(1);

    return !resp;
}

void OneWire::write_bit(bool bit) {
    pin.write(1);
    pin.output(0);

    if (bit) {
        delay_us(8);
    } else {
        delay_us(80);
    }

    pin.output(1);

    if (bit) {
        delay_us(80);
    } else {
        delay_us(8);
    }
}

bool OneWire::read_bit() {
    pin.write(1);
    pin.output(0);

    delay_us(2);
    pin.direction(Gpio::Input);

    delay_us(5);
    bool bit = pin.get();

    delay_us(60);
    return bit;
}

void OneWire::write_byte(uint8_t byte) {
    for (int i=0; i<8; i++) {
        write_bit(byte & (1 << i));
    }
}

uint8_t OneWire::read_byte() {
    uint8_t byte = 0;

    for (int i=0; i<8; i++) {
        byte |= (uint8_t) read_bit() << i;
    }

    return byte;
}

uint8_t Ds18b20::crc8(uint8_t* data, uint8_t len) {
    uint8_t crc = 0;

    for (uint8_t i = 0; i < len; i++) {
        uint8_t byte = data[i];

        for (uint8_t j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ byte) & 0x01;

            crc = crc >> 1;
            if (mix) {
                crc ^= 0x8C;
            }

            byte = byte >> 1;
        }
    }

    return crc;
}

bool Ds18b20::read_scratch_pad() {
    bool ok = reset();
    if (!ok) return false;

    write_byte(COMMAND_SKIP_ROM);
    
    write_byte(COMMAND_READ_SP);
    for (int i=0; i<9; i++) {
        scratch_pad[i] = read_byte();
    }

    return crc8(scratch_pad, 8) == scratch_pad[8];
}

bool Ds18b20::start_conversion() {
    bool ok = reset();
    if (!ok) return false;

    write_byte(COMMAND_SKIP_ROM);
    write_byte(COMMAND_START_CONVERSION);
    return true;
}

uint16_t Ds18b20::get_temperature() {
    bool ok = read_scratch_pad();
    if (!ok) return 0x7FFF;

    return scratch_pad[0] + (uint16_t)(scratch_pad[1] << 8);
}

