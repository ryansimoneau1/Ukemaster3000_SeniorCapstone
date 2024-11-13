#include <F28x_Project.h>
#include <float.h>
#include <stdio.h>
#include "F2837xD_device.h"
#include "device.h"
void InitEPwm1(void);
void InitEPwm1(void)
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // Set GPIO0 as PWM output
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;  // Set GPIO0 as output
    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1; // Set initial output to low
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_FREEZE; // Freeze counter
    EPwm1Regs.TBPRD = 17; // Set period to 40 (40 MHz / 40 = 1 MHz)
    EPwm1Regs.CMPA.bit.CMPA = 10; // Set duty cycle to 30% (12 / 40 = 30%)
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET; // Set output to high on counter zero
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR; // Set output to low on compare match
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up-down mode
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE; // Disable phase loading
    EPwm1Regs.TBPHS.bit.TBPHS = 0x0000; // Phase is 0
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1; // HSPCLK = SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1; // TBCLK = HSPCLK
}


void main(){

    InitSysCtrl();

    InitEPwm1();

    while(1);
}
