#ifndef USB_H
#define USB_H

#include <array>

#include "debug.h"

#include "sam.h"
#include "usb_protocol.h"
#include "usb_endpoint.h"

constexpr uint8_t USB_NUM_ENDPOINTS = 2;

extern InterruptEndpoint app_out;
extern InterruptEndpoint app_in;

class UsbController {
public:
    UsbController();

    void initialize(uint8_t speed);
    void attach();
    void detach();
    void reset();
    bool is_attached();

    void set_address(uint8_t addr=0);
    uint8_t get_configuration() { return 0; }
    bool set_configuration(uint16_t value);
    bool set_interface(uint16_t index, uint16_t value);

    UsbDeviceDescriptor* get_descriptor(uint8_t num);
    std::array<UsbEndpoint*, USB_NUM_ENDPOINTS> endpoints{{&app_out, &app_in}};

    ControlEndpoint ep0;

private:
    UsbDeviceDescriptor ep_descriptors[USB_NUM_ENDPOINTS + 1];
    uint8_t address;
};

extern UsbController usb;

#endif /* USB_H */