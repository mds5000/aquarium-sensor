#include "usb_controller.h"

#include <cstring>

#define NVM_CAL ((uint32_t*)(0x00806020UL))
#define USB_TRANSN_MASK     0x0003E000
#define USB_TRANSN_SHIFT    13

#define USB_TRANSP_MASK     0x007C0000
#define USB_TRANSP_SHIFT    18

#define USB_TRIM_MASK       0x03800000
#define USB_TRIM_SHIFT      23

UsbController usb;

UsbController::UsbController() {
    /* Clear endpoint memory */
    std::memset(ep_descriptors, 0, (USB_NUM_ENDPOINTS+1)*sizeof(UsbDeviceDescriptor));

    /* Setup SOF 1khz I/O */
    PORT->Group[0].PMUX[11].bit.PMUXO = 6;
    PORT->Group[0].PINCFG[23].bit.PMUXEN = 1;

    /* Setup USB_DM/USB_DP I/O */
    PORT->Group[0].PMUX[12].reg = PORT_PMUX_PMUXE(6) | PORT_PMUX_PMUXO(6);
    PORT->Group[0].PINCFG[24].bit.PMUXEN = 1;
    PORT->Group[0].PINCFG[25].bit.PMUXEN = 1;
}

void UsbController::initialize(uint8_t speed) {
    /* Load USB Pad Calibration Values */
    uint16_t pad_transn = (*NVM_CAL & USB_TRANSN_MASK) >> USB_TRANSN_SHIFT;
    uint16_t pad_transp = (*NVM_CAL & USB_TRANSP_MASK) >> USB_TRANSP_SHIFT;
    uint16_t pad_trim = (*NVM_CAL & USB_TRIM_MASK) >> USB_TRIM_SHIFT;

    USB->DEVICE.PADCAL.reg = USB_PADCAL_TRANSN(pad_transn) |
                             USB_PADCAL_TRANSP(pad_transp) |
                             USB_PADCAL_TRIM(pad_trim);

    USB->DEVICE.QOSCTRL.bit.CQOS = 3;
    USB->DEVICE.QOSCTRL.bit.DQOS = 3;

    USB->DEVICE.CTRLB.bit.SPDCONF = speed;
	USB->DEVICE.DESCADD.reg = (uint32_t)(&ep_descriptors[0]);

    reset();
    debug("USB Initialization complete.");
}

void UsbController::detach(void) {
    debug("USB Detached...");
	USB->DEVICE.CTRLB.bit.DETACH = 1;
	NVIC_DisableIRQ(USB_IRQn);
    reset();
}

void UsbController::attach(void) {
    debug("USB Attached...");
	NVIC_EnableIRQ(USB_IRQn);
	USB->DEVICE.CTRLB.bit.DETACH = 0;
}

bool UsbController::is_attached() { 
    return USB->DEVICE.CTRLB.bit.DETACH == 0;
}

UsbDeviceDescriptor* UsbController::get_descriptor(uint8_t num) {
    return &usb.ep_descriptors[num];
}

void UsbController::reset() {
    debug("USB Reset...");
    /* Reset Endpoints */
    ep0.reset();
    for (int i = 0; i < USB_NUM_ENDPOINTS; i++) {
    //    endpoints[i]->reset();
    }

    ep0.enable();
    ep0.enable_setup();

	USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_EORST;
    USB->DEVICE.CTRLA.bit.ENABLE = 1;
}

/**
 * USB Interrupt Handler
 **/
void USB_Handler() {
    uint32_t flags = USB->DEVICE.INTFLAG.reg;
    uint32_t epflags = USB->DEVICE.EPINTSMRY.reg;
    debug("USB-INT: 0x%x/0x%x", flags, epflags);

    /* USB End-of-Reset Interrupt */
    if (flags & USB_DEVICE_INTFLAG_EORST) {
        USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTENCLR_EORST;
        usb.reset();
    }

    /* Handle Control Endpoint */
    if (epflags & 1) {
		uint32_t flags = USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg;
		USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1
                                                    | USB_DEVICE_EPINTFLAG_TRCPT0
                                                    | USB_DEVICE_EPINTFLAG_RXSTP;
        if (flags & USB_DEVICE_EPINTFLAG_TRFAIL0)
            debug("EP0: 0x%x: TRFAIL 0", flags);
        if (flags & USB_DEVICE_EPINTFLAG_TRFAIL1)
            debug("EP0: 0x%x: TRFAIL 1", flags);


        debug("EP0: 0x%x", flags);

		if (flags & USB_DEVICE_EPINTFLAG_RXSTP) {
            usb.ep0.handle_setup();
        }

        if (flags & USB_DEVICE_EPINTFLAG_TRCPT1 && !(flags & USB_DEVICE_EPINTFLAG_TRFAIL1)) {
            if (usb.ep0.address) {
                usb.ep0.commit_address();
            }
        }
    }

    /* Handle Application Endpoints */
    for (int i = 1; i <= USB_NUM_ENDPOINTS; i++) {
        if (epflags & 1 << i) {
            uint32_t flags = USB->DEVICE.DeviceEndpoint[i].EPINTFLAG.reg;
            USB->DEVICE.DeviceEndpoint[i].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1
                                                        | USB_DEVICE_EPINTFLAG_TRCPT0
                                                        | USB_DEVICE_EPINTFLAG_RXSTP;
            debug("EP%d: 0x%x", i, flags);

            if (flags & USB_DEVICE_EPINTFLAG_TRCPT0 ) {
                debug("OUT_%d", i);
            }

            if (flags & USB_DEVICE_EPINTFLAG_TRCPT1 ) {
                debug("IN_%d", i);
            }
        }
    }
}