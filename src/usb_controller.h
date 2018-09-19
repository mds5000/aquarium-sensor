#ifndef USB_H
#define USB_H

#include <vector>

#include "debug.h"

#include "sam.h"
#include "usb_protocol.h"
#include "usb_endpoint.h"

constexpr uint8_t USB_NUM_ENDPOINTS = 2;

class UsbController {
public:
    UsbController();

    void initialize(const UsbSpeed speed);
    void add_endpoint(uint8_t number, uint8_t type, uint16_t size);
    void attach();
    void detach();
    void reset();

    void commit_address();

    ControlEndpoint* ep0;
private:
    UsbDeviceDescriptor ep_desciptors[USB_NUM_ENDPOINTS];
    std::vector<UsbEndpoint*> endpoints;

    uint8_t address;


};

extern UsbController usb;

#endif /* USB_H */