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
    std::memset(ep_desciptors, 0, USB_NUM_ENDPOINTS*sizeof(UsbDeviceDescriptor));
    ep0 = new ControlEndpoint(&ep_desciptors[0]);

    /* Setup SOF 1khz I/O */
    PORT->Group[0].PMUX[11].bit.PMUXO = 6;
    PORT->Group[0].PINCFG[23].bit.PMUXEN = 1;

    /* Setup USB_DM/USB_DP I/O */
    PORT->Group[0].PMUX[12].reg = PORT_PMUX_PMUXE(6) | PORT_PMUX_PMUXO(6);
    PORT->Group[0].PINCFG[24].bit.PMUXEN = 1;
    PORT->Group[0].PINCFG[25].bit.PMUXEN = 1;
}

void UsbController::initialize(const UsbSpeed speed) {
    /* Load USB Pad Calibration Values */
    uint16_t pad_transn = (*NVM_CAL & USB_TRANSN_MASK) >> USB_TRANSN_SHIFT;
    uint16_t pad_transp = (*NVM_CAL & USB_TRANSP_MASK) >> USB_TRANSP_SHIFT;
    uint16_t pad_trim = (*NVM_CAL & USB_TRIM_MASK) >> USB_TRIM_SHIFT;

    USB->DEVICE.PADCAL.reg = USB_PADCAL_TRANSN(pad_transn) |
                             USB_PADCAL_TRANSP(pad_transp) |
                             USB_PADCAL_TRIM(pad_trim);

    USB->DEVICE.QOSCTRL.bit.CQOS = 3;
    USB->DEVICE.QOSCTRL.bit.DQOS = 3;

    USB->DEVICE.CTRLB.bit.SPDCONF = (uint8_t)speed;
	USB->DEVICE.DESCADD.reg = (uint32_t)(&ep_desciptors[0]);

    /* Reset & Enable */
    reset();
    attach();
}

void UsbController::detach(void) {
	USB->DEVICE.CTRLB.bit.DETACH = 1;
	NVIC_DisableIRQ(USB_IRQn);
}

void UsbController::attach(void) {
	NVIC_EnableIRQ(USB_IRQn);
	USB->DEVICE.CTRLB.bit.DETACH = 0;
}

void UsbController::reset() {
    /* Reset Endpoints */
    ep0->reset();
    for (auto& ep : endpoints) {
        ep->reset();
    }

    ep0->enable();
    ep0->enable_setup();

	USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_EORST;
    USB->DEVICE.CTRLA.bit.ENABLE = 1;
}

void UsbController::commit_address() {
    debug("Updated USB Device Address (%d)", ep0->address);
    USB->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | USB_DEVICE_DADD_DADD(ep0->address);
    ep0->address = 0;
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
		USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1 | USB_DEVICE_EPINTFLAG_TRCPT0 | USB_DEVICE_EPINTFLAG_RXSTP;
        debug("EP0: 0x%x", flags);

		if (flags & USB_DEVICE_EPINTFLAG_RXSTP) {
            usb.ep0->handle_setup();
        }

        if (flags & USB_DEVICE_EPINTFLAG_TRCPT0 ) {
            debug("OUT_0");
        }

        if (flags & USB_DEVICE_EPINTFLAG_TRCPT1 ) {
            if (usb.ep0->address) {
                usb.commit_address();
            }
            debug("IN_0");
        }
    }

    /* Handle Application Endpoints */
}