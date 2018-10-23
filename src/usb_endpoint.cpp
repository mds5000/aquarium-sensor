#include "usb_endpoint.h"

#include <cstring>

#include "usb_conf.h"
#include "usb_controller.h"

extern UsbController usb;


UsbEndpoint::UsbEndpoint(uint8_t number_, uint8_t type_, uint16_t size) 
  : number(number_ & 0x3f), ep_type(type_), buffer_size(size)
{
    dev = &USB->DEVICE.DeviceEndpoint[number];
    out_desc = &usb.get_descriptor(number)->DeviceDescBank[0];
    in_desc = &usb.get_descriptor(number)->DeviceDescBank[1];

    debug("EP%d: %x, %x, %x", number, dev, out_desc, in_desc);
}

void UsbEndpoint::reset() {
    dev->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_BK1RDY;
    dev->EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_BK0RDY;
}

void UsbEndpoint::enable() {
    in_desc->PCKSIZE.bit.SIZE = calc_size(buffer_size);
    dev->EPCFG.bit.EPTYPE1 = ep_type + 1;
    dev->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_BK1RDY
                         | USB_DEVICE_EPSTATUS_STALLRQ(0x2)
                         | USB_DEVICE_EPSTATUS_DTGLIN;

    out_desc->PCKSIZE.bit.SIZE = calc_size(buffer_size);
    dev->EPCFG.bit.EPTYPE0 = ep_type + 1;
    dev->EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_BK0RDY;
    dev->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_STALLRQ(0x1)
                         | USB_DEVICE_EPSTATUS_DTGLOUT;
}

void UsbEndpoint::start_out(char* data_dest, int len) {
    out_desc->PCKSIZE.bit.MULTI_PACKET_SIZE = len;
    out_desc->PCKSIZE.bit.BYTE_COUNT = 0;
    out_desc->ADDR.reg = (uint32_t)data_dest;

    dev->EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT0 | USB_DEVICE_EPINTFLAG_TRFAIL0;
    dev->EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0;
    dev->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUS_BK0RDY;
}

void UsbEndpoint::start_in(const char* data_src, int len, bool zlp) {
    in_desc->PCKSIZE.bit.AUTO_ZLP = zlp;
    in_desc->PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
    in_desc->PCKSIZE.bit.BYTE_COUNT = len;
    in_desc->ADDR.reg = (uint32_t)data_src;

    dev->EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1 | USB_DEVICE_EPINTFLAG_TRFAIL1;
    dev->EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT1;
    dev->EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_BK1RDY;
}

void UsbEndpoint::start_stall() {
	dev->EPSTATUSSET.reg = USB_DEVICE_EPSTATUS_STALLRQ(0x3);
}

uint8_t UsbEndpoint::calc_size(int size) {
    if (size <= 8) return 0;
    if (size <= 16) return 1;
    if (size <= 32) return 2;
    if (size <= 64) return 3;
    if (size <= 128) return 4;
    if (size <= 256) return 5;
    if (size <= 512) return 6;
    return 7;
}

void ControlEndpoint::enable_setup() {
    address = 0;
    dev->EPINTENSET.reg = USB_DEVICE_EPINTENSET_RXSTP;
}

void ControlEndpoint::handle_setup() {
    UsbSetupPacket setup_packet;
    std::memcpy(&setup_packet, ep_out, sizeof(UsbSetupPacket));

    debug("USB SETUP: req:%d, typ:%d, val:%d, idx:%d, len:%d",
        setup_packet.bmRequestType, setup_packet.bRequest, setup_packet.wValue,
        setup_packet.wIndex, setup_packet.wLength);

    if ((setup_packet.bmRequestType & USB_REQTYPE_TYPE_MASK) != USB_REQTYPE_STANDARD) {
        debug("NONSTANDARD USB REQEST");
        return;
    }

    int len = 0;
    switch (setup_packet.bRequest) {
        case USB_REQ_GetStatus:
            start_in("\x00\x00", 2);
            start_out();
            return;
        case USB_REQ_ClearFeature:
        case USB_REQ_SetFeature:
        case USB_REQ_SetAddress:
            address = setup_packet.wValue & 0x7F;
            start_in(NULL, 0);
            start_out();
            return;
        case USB_REQ_GetDescriptor:
            len = get_descriptor(setup_packet.wValue);
            if (len <= 0) {
                start_stall();
                return;
            }
            start_in(ep_in, len);
            start_out();
            debug("Sent Descriptor: %d", len);
            return;
        case USB_REQ_GetConfiguration:
            ep_in[0] = (char)get_configuration();
            start_in(ep_in, 1);
            start_out();
            return;
        case USB_REQ_SetConfiguration:
            if (!set_configuration(setup_packet.wValue)) {
                start_stall();
                return;
            }
            start_in(NULL, 0);
            start_out();
            return;
        case USB_REQ_SetInterface:
            if (!set_interface(setup_packet.wIndex, setup_packet.wValue)) {
                start_stall();
                return;
            }
            start_in(NULL, 0);
            start_out();
            return;
    }
}

void ControlEndpoint::commit_address() {
    debug("Updated USB Device Address (%d)", address);
    USB->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | USB_DEVICE_DADD_DADD(address);
    address = 0;
}

uint8_t ControlEndpoint::generate_string_descriptor(uint16_t index) {
    const char* string = usb_string_descriptor_table[index];
    uint8_t len = std::strlen(string);

    ep_in[0] = (len)*2 + 2;
    ep_in[1] = 0x03;

    uint16_t* ep_ptr = (uint16_t*)ep_in + 1;
    for (int i=0; i<len; i++) {
        ep_ptr[i] = string[i];
    }

    return ep_in[0];
}

uint16_t ControlEndpoint::get_descriptor(uint16_t value) {
    uint8_t type = value >> 8;
    uint8_t index = value & 0xFF;

    debug("desc: %d - %d", type, index);

    switch(type) {
        case USB_DTYPE_Device:
            std::memcpy(ep_in, &usb_device_descriptor, sizeof(UsbDeviceProtocolDescriptor));
            return sizeof(UsbDeviceProtocolDescriptor);

        case USB_DTYPE_Configuration:
            std::memcpy(ep_in, &usb_config_descriptor, sizeof(UsbConfigurationDescriptor));
            std::memcpy(ep_in + sizeof(UsbConfigurationDescriptor),
                        &usb_interface_descriptor, sizeof(UsbInterfaceDescriptor));
            std::memcpy(ep_in + sizeof(UsbConfigurationDescriptor)
                              + sizeof(UsbInterfaceDescriptor),
                        &usb_out_endpoint_descriptor, sizeof(UsbEndpointDescriptor));
            std::memcpy(ep_in + sizeof(UsbConfigurationDescriptor)
                              + sizeof(UsbInterfaceDescriptor)
                              + sizeof(UsbEndpointDescriptor),
                        &usb_in_endpoint_descriptor, sizeof(UsbEndpointDescriptor));
            return sizeof(UsbConfigurationDescriptor) +
                   sizeof(UsbInterfaceDescriptor) +
                   2 * sizeof(UsbEndpointDescriptor);

        case USB_DTYPE_Interface:
            std::memcpy(ep_in, &usb_interface_descriptor, sizeof(UsbInterfaceDescriptor));
            return sizeof(UsbInterfaceDescriptor);

        case USB_DTYPE_Endpoint:
            if (index == 0)
                std::memcpy(ep_in, &usb_out_endpoint_descriptor, sizeof(UsbEndpointDescriptor));
            else
                std::memcpy(ep_in, &usb_in_endpoint_descriptor, sizeof(UsbEndpointDescriptor));
            return sizeof(UsbEndpointDescriptor);

        case USB_DTYPE_String:
            if (index == 0) {
                std::memcpy(ep_in, &usb_string_descriptor, usb_string_descriptor.bLength);
                return usb_string_descriptor.bLength;
            }
            return generate_string_descriptor(index);

        default:
            return 0;
    }
}
