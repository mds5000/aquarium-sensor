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

    void enable();
    void reset();

    void handle_out();
    void handle_in();

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

    int pending_out_bytes;
};


class ControlEndpoint : public UsbEndpoint {
public:
    ControlEndpoint()
    : UsbEndpoint(0, USB_EP_TYPE_CONTROL, EP_SIZE) {}

    void enable_setup();
    void handle_setup();

    uint16_t get_descriptor(uint16_t value);
    uint8_t generate_string_descriptor(uint16_t index);
};

class InterruptEndpoint : public UsbEndpoint {
public:
    InterruptEndpoint(uint8_t number) 
    : UsbEndpoint(number, USB_EP_TYPE_INTERRUPT, EP_SIZE) {}

    bool pending() { return pending_out_bytes > 0; }
    //bool write(char* message, int len);
    //int read(char* dest);

};

#endif /* USB_ENDPOINT_H */