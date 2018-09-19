#ifndef USB_ENDPOINT_H
#define USB_ENDPOINT_H

#include "debug.h"

#include "sam.h"
#include "usb_protocol.h"

class UsbEndpoint {
public:
    UsbEndpoint(UsbDeviceDescriptor *ep_base, uint8_t number, uint8_t type, const uint16_t size);

    void start_out(int len=0);
    void start_in(int len, bool zlp=true);
    void start_stall();

    void enable();
    void reset();

    bool ready();
    bool pending();

protected:
    static uint8_t calc_size(int size);

    /* Endpoint Number */
    const uint8_t number;
    const uint8_t ep_type;
    const int buffer_size;

    /* Const pointer to a volatile endpoint registers */
    UsbDeviceEndpoint volatile * dev;

    /* Const pointer to Endpoint Desciptors */
    UsbDeviceDescBank * out_desc;
    UsbDeviceDescBank * in_desc;
    
    char *ep_in;
    char *ep_out;
};


class ControlEndpoint : public UsbEndpoint {
public:
    ControlEndpoint(UsbDeviceDescriptor *ep_base) 
    : UsbEndpoint(ep_base, 0, USB_EP_TYPE_CONTROL, 64) {}

    void enable_setup();
    void handle_setup();

    bool set_configuration(uint16_t value) { return true; }
    bool set_interface(uint16_t index, uint16_t value) { return true; }

    uint16_t get_descriptor(uint16_t value);
    uint8_t get_configuration() { return 0; }

    uint8_t generate_string_descriptor(uint16_t index);

    uint8_t address;

};

#endif /* USB_ENDPOINT_H */