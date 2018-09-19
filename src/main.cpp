/*
 * $projectname$.cpp
 *
 * Created: $date$
 * Author : $user$
 */ 

#include "debug.h"

#include "sam.h"

#include "board.h"
#include "temp_sensor.h"
#include "usb_controller.h"


#define NVM_CAL ((uint32_t*)(0x00806020UL))
#define ADC_BIASREF_MASK    0x00000007
#define ADC_BIASREF_SHIFT   0

#define ADC_BIASCOMP_MASK   0x00000038
#define ADC_BIASCOMP_SHIFT  3

#define DFLL_CAL_MASK       0xFC000000
#define DFLL_CAL_SHIFT      26


void initialize_system() {
    /* Set NVRAM Waitstaits */
    NVMCTRL->CTRLB.bit.RWS = 0;

    /* Set Performance Level 2 */
    PM->PLCFG.bit.PLSEL = 2;

    /* OSC to 16MHz */
    OSCCTRL->OSC16MCTRL.reg = 0x0E;

    /* DFLL Locked to USB SOF */
    OSCCTRL->DFLLCTRL.bit.ONDEMAND = 0;
    OSCCTRL->DFLLCTRL.bit.ENABLE = 1;
    uint32_t coarse_val = (*NVM_CAL & DFLL_CAL_MASK) >> DFLL_CAL_SHIFT;

    while (!OSCCTRL->STATUS.bit.DFLLRDY) {};
    OSCCTRL->DFLLVAL.reg = OSCCTRL_DFLLVAL_COARSE(coarse_val) |
                           OSCCTRL_DFLLVAL_FINE(512);

    while (!OSCCTRL->STATUS.bit.DFLLRDY) {};
    OSCCTRL->DFLLMUL.reg = OSCCTRL_DFLLMUL_CSTEP(1) |
                           OSCCTRL_DFLLMUL_FSTEP(1) |
                           OSCCTRL_DFLLMUL_MUL(48000);

    OSCCTRL->DFLLCTRL.reg = OSCCTRL_DFLLCTRL_CCDIS |
                            OSCCTRL_DFLLCTRL_USBCRM |
                            OSCCTRL_DFLLCTRL_MODE |
                            OSCCTRL_DFLLCTRL_ENABLE;

    OSCCTRL->DFLLSYNC.bit.READREQ = 1;
    debug("NVM_CAL: 0x%x, 0x%x", *NVM_CAL, coarse_val);
    debug("DFLLCTRL: 0x%x", OSCCTRL->DFLLCTRL.reg);
    debug("DFLLSTATUS: 0x%x", OSCCTRL->STATUS.reg);
    debug("DFLLMUL: 0x%x", OSCCTRL->DFLLMUL.reg);
    debug("DFLLVAL: 0x%x", OSCCTRL->DFLLVAL.reg);

    while (!OSCCTRL->STATUS.bit.DFLLRDY) {};

    /* Configure MCLK */
    MCLK->BUPDIV.bit.BUPDIV = 8;
    MCLK->LPDIV.bit.LPDIV = 4;
    MCLK->CPUDIV.bit.CPUDIV = 1;

    /* Configure GCLKs */
    GCLK->GENCTRL[0].reg = 0x00000106;
    GCLK->GENCTRL[1].reg = 0x00000107;

    // H mode on pin 13, PORTA10
    GCLK->GENCTRL[4].reg = 0x000A0907;
    PORT->Group[0].PMUX[5].bit.PMUXE = 7;
    PORT->Group[0].PINCFG[10].bit.PMUXEN = 1;

    /* Configure Peripheral Clocks */
    //GCLK->PCHCTRL[0].reg = GCLK_PCHCTRL_GEN_GCLK3 | GCLK_PCHCTRL_CHEN;
    /* Enable USB Peripheral CLK */
    GCLK->PCHCTRL[4].reg = GCLK_PCHCTRL_GEN_GCLK1 | GCLK_PCHCTRL_CHEN;

    /* Confiugre LED I/O */
    led_green.direction(true);
    led_red.direction(true);
    led_green.output(1);

}

void die() {
    while (1) {
        led_red.output(1);
        delay_us(50000);
        led_red.output(0);
        delay_us(50000);
    }
}

int main(void)
{
    SEGGER_RTT_WriteString(0, "\r\n\r\nSTART:\r\n");
    /* Initialize the SAM system */
    SystemInit();
    initialize_system();

    usb.initialize(USB_SPEED_LOW);

    SEGGER_RTT_printf(0, "USB: %x\r\n", USB->DEVICE.CTRLA.reg);
    SEGGER_RTT_printf(0, "USB-FSM: %x\r\n", USB->DEVICE.FSMSTATUS.reg);


    auto temp = Ds18b20(gpio_temp);
    temp.start_conversion();
    delay_ms(1000);
    auto val = temp.get_temperature();

    int x = (val & 0x0F) * 625;
    SEGGER_RTT_printf(0, "OK: %d.%04d", val/16, x);

    die();
}
