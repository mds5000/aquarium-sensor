#ifndef USB_H
#define USB_H

#include <array>

#include "debug.h"

#include "sam.h"
#include "usb_protocol.h"
#include "usb_endpoint.h"

constexpr uint8_t USB_NUM_ENDPOINTS = 3;

extern ControlEndpoint ep0;
extern InterruptEndpoint app_in;
extern InterruptEndpoint app_out;

class UsbController {
public:
    UsbController();

    void initialize(uint8_t speed);
    void attach();
    void detach();
    void reset();
    bool is_attached();

    UsbDeviceDescriptor* get_descriptor(uint8_t num);
    std::array<UsbEndpoint*, USB_NUM_ENDPOINTS> endpoints{{&ep0, &app_in, &app_out}};

private:
    UsbDeviceDescriptor ep_descriptors[USB_NUM_ENDPOINTS];
};

extern UsbController usb;

#endif /* USB_H */