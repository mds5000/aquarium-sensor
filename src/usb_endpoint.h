#ifndef USB_ENDPOINT_H
#define USB_ENDPOINT_H

#include "debug.h"

#include "sam.h"
#include "usb_protocol.h"

constexpr auto EP_SIZE = 64;


class UsbEndpoint {
public:
    UsbEndpoint(uint8_t number, uint8_t type, const uint16_t size);

    void start_out(int len=0);
    void start_in(int len, bool zlp=true);
    void start_stall();

    virtual void enable();
    void reset();

    virtual void handle_out(char* message, int len) {
        debug("USB EP: ");
        SEGGER_RTT_Write(0, message, len);
    }

protected:
    static uint8_t calc_size(int size);

    const uint8_t ep_type;
    const int buffer_size;

    /* Const pointer to a volatile endpoint registers */
    volatile UsbDeviceEndpoint * const dev;

    /* Const pointer to Endpoint Desciptors */
    UsbDeviceDescBank * const out_desc;
    UsbDeviceDescBank * const in_desc;

    char ep_in[EP_SIZE] __attribute__ ((aligned (4)));
    char ep_out[EP_SIZE] __attribute__ ((aligned (4)));
};


class ControlEndpoint : public UsbEndpoint {
public:
    ControlEndpoint()
    : UsbEndpoint(0, USB_EP_TYPE_CONTROL, EP_SIZE) {}

    void enable() override;
    void handle_setup();
    void commit_address();

    bool set_configuration(uint16_t value) { return true; }
    bool set_interface(uint16_t index, uint16_t value) { return true; }

    uint16_t get_descriptor(uint16_t value);
    uint8_t get_configuration() { return 0; }

    uint8_t generate_string_descriptor(uint16_t index);

    uint8_t address;
};

<<<<<<< HEAD
class InterruptEndpoint : public UsbEndpoint {
public:
    InterruptEndpoint(uint8_t number) 
    : UsbEndpoint(number, USB_EP_TYPE_INTERRUPT, EP_SIZE) {}
=======
    char ep_in[CONTROL_EP_SIZE] __attribute__ ((aligned (4)));
    char ep_out[CONTROL_EP_SIZE] __attribute__ ((aligned (4)));
>>>>>>> c14af423f04b389c4fbc8f03d6ac11209586ee86
};

#endif /* USB_ENDPOINT_H */