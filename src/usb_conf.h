
#include "usb_protocol.h"

UsbDeviceProtocolDescriptor usb_device_descriptor{
    .bLength = sizeof(UsbDeviceProtocolDescriptor),
    .bDescriptorType = 0x01,
    .bcdUSB = 0x0110, /* USB 1.1 */
    .bDeviceClass = 0, /* Interface Defined */
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize = 64,
    .idVendor = 0x1209,
    .idProduct = 0x1986,
    .bcdDevice = 0x0100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1
};

UsbConfigurationDescriptor usb_config_descriptor{
    .bLength = sizeof(UsbConfigurationDescriptor),
    .bDescriptorType = 0x02,
    .wTotalLength = sizeof(UsbConfigurationDescriptor)
                  + sizeof(UsbInterfaceDescriptor)
                  + 2 * sizeof(UsbEndpointDescriptor),
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 4,
    .bmAttributes = 0xC0, /* Self-Powered */
    .bMaxPower = 50       /* 100mA */
};

UsbInterfaceDescriptor usb_interface_descriptor{
    .bLength = sizeof(UsbInterfaceDescriptor),
    .bDescriptorType = 0x04,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = 0xFF,
    .bInterfaceSubClass = 0xFF,
    .bInterfaceProtocol = 0xFF,
    .iInterface = 5 
};

UsbEndpointDescriptor usb_out_endpoint_descriptor{
    .bLength = sizeof(UsbEndpointDescriptor),
    .bDescriptorType = 0x05,
    .bEndpointAddress = 0x01,
    .bmAttributes = 0x3, /* Interrupt Endpoint */
    .wMaxPacketSize = 64,
    .bInterval = 1
};

UsbEndpointDescriptor usb_in_endpoint_descriptor{
    .bLength = sizeof(UsbEndpointDescriptor),
    .bDescriptorType = 0x05,
    .bEndpointAddress = 0x81,
    .bmAttributes = 0x3, /* Interrupt Endpoint */
    .wMaxPacketSize = 64,
    .bInterval = 1
};

UsbString0Descriptor usb_string_descriptor{
    .bLength = 4,
    .bDescriptorType = 0x03,
    .language = 0x0409
};

const char* usb_string_descriptor_table[] = {
    "",
    "MyManufacturer",
    "MyProduct",
    "MySerial",
    "Configuration",
    "Interface"
};


