#ifndef USB_H
#define USB_H

#include <array>

#include "debug.h"

#include "sam.h"
#include "usb_protocol.h"
#include "usb_endpoint.h"

constexpr uint8_t USB_NUM_ENDPOINTS = 2;

class UsbController {
public:
    UsbController();

    void initialize(uint8_t speed);
    void attach();
    void detach();
    void reset();
    bool is_attached();

    UsbDeviceDescriptor* get_descriptor(uint8_t num);

    ControlEndpoint ep0;
    UsbEndpoint* endpoints[USB_NUM_ENDPOINTS];

private:
    UsbDeviceDescriptor ep_descriptors[USB_NUM_ENDPOINTS+1];
};

extern UsbController usb;

#endif /* USB_H */