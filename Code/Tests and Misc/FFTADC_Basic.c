//#############################################################################
//
// FILE:        main.c
//
// Description: This example shows how to use the C2000 Real-Time CLA
//              complex FFT algorithms to perform a real FFT
//
//#############################################################################
//
//
// $Copyright: Copyright (C) 2023 Texas Instruments Incorporated -
//             http://www.ti.com/ ALL RIGHTS RESERVED $
//#############################################################################

//*****************************************************************************
// includes
//*****************************************************************************
#include "f2837x_fft_examples_setup.h"
#include <F28x_Project.h>
#include "F2837xD_device.h"
#include "rfft_1024_shared.h"
#include "device.h"
#include <float.h>
#include <stdio.h>
#include <math.h>
#include "dma.h"

//!
//! \addtogroup RFFT_EXAMPLES Real Fast Fourier Transform (N = 1024) Example

// @{

//*****************************************************************************
// defines
//*****************************************************************************
#define NSTAGES         9               // 512
#define NSAMPLES        (1<<NSTAGES)
#define TRANSFER        1024
#define BURST           1
//*****************************************************************************
// globals
//*****************************************************************************
#ifdef __cplusplus
#pragma DATA_SECTION("IOBuffer")
#else
#pragma DATA_SECTION(IOBuffer,"IOBuffer")
#endif //__cplusplus
// \brief Test Input Data
//
float IOBuffer[2*NSAMPLES];

#ifdef __cplusplus
#pragma DATA_SECTION("IOBuffer")
#else
#pragma DATA_SECTION(IOBuffer2,"IOBuffer")
#endif //__cplusplus
// \brief Output of the unpack routine
//
float IOBuffer2[2*NSAMPLES + 2];

Uint16 FFTMag[2*NSAMPLES + 2];
Uint16 ADCBuff[2*NSAMPLES];
Uint16 TestBuff[2*NSAMPLES];
//*****************************************************************************
// Function Prototypes
//*****************************************************************************
void ConfigureADC(void);
void InitTimer1(void);
interrupt void ADC_ISR(void);
//*****************************************************************************
// function definitions
//*****************************************************************************
void InitTimer1(void) {
    InitCpuTimers();                            // Initialize all timers to known state
    ConfigCpuTimer(&CpuTimer1, 100, 100);   // Configure CPU timer 1. 200 -> SYSCLK in MHz, 100us -> period in usec. NOTE: Does NOT start timer
    CpuTimer1.RegsAddr->TCR.bit.TSS = 0;        // Start timer 1
}

void ConfigureADC(void)
{
    //Set Up interrupts for ADC
    PieVectTable.ADCA1_INT = &ADC_ISR; // say where the ISR is
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // PIE group 1, interrupt 1 -> ADCA1
    IER |= M_INT1;

    // CTL Regs
    AdcaRegs.ADCCTL2.bit.PRESCALE = 0xA;                               // Set ADCCLK to SYSCLK/64
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE); // Initializes ADCA to 16-bit and single-ended mode. Performs internal calibration
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;                                 // Powers up ADC
    DELAY_US(1000);                                                    // Delay to allow ADC to power up
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 0;                              // Occurs at the end of the conversion

    // SOC Regs
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 2;                               // Use CPU CLK 1 SOC as trigger source
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 1;                                 // Sets SOC0 to channel 1 -> pin ADCINA1
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 100;                               // Sets sample and hold window.
    AdcaRegs.ADCINTSOCSEL1.bit.SOC0 = 0;                               // SOC not triggered by ePWM. (should say when sample is ready)

    // Interrupt CTL Regs
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0;                             // continuous mode
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;                               // Enable ADCINT1 interrupt
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;                             // Clear ADC interrupt flag
}


Uint16 ADCCnt = 0; // ADC sample #
Uint16 FFTflag = 0; // sets high when #samples = 1024
Uint16 index;
Uint16 ADCcnt = 0;
Uint16 Debug = 0;
int main(void)
{

    //
    // Clear out the final result buffer
    //
    for(index = 0; index < 2*NSAMPLES + 2  ; index++)
    {
        IOBuffer2[index] = 0.0;
    }

//    InitSysCtrl();
    CLA_DSP_initSystemClocks();
    CLA_DSP_configClaMemory();
    CLA_DSP_initCpu1Cla1();

    EALLOW;             // allows ADC Registers to be modified

    GpioCtrlRegs.GPADIR.bit.GPIO19 = 1; // Set pin as output (pin 3 on launchpad)

    DINT;               // Disable CPU interrupts on startup

    // Init PIE
    InitPieCtrl();      // Initialize PIE -> disable PIE and clear all PIE registers
    IER = 0x0000;       // Clear CPU interrupt register
    IFR = 0x0000;       // Clear CPU interrupt flag register
    InitPieVectTable(); // Initialize PIE vector table to known state

    EALLOW;
//    initDMA();
    InitTimer1();
    ConfigureADC();

    EnableInterrupts(); // Enable PIE and CPU interrupts. Includes EINT

    EDIS; // disable Editing of control registers


    while(1){

        if(FFTflag == 1){

            asm("  IACK  #0x0001");
            asm("  RPT #3 || NOP");
            while (CLA_getTaskRunStatus(CLA1_BASE, CLA_TASK_1) == 1);

            FFTflag = 0;

            for(index = 0; index < 2*NSAMPLES + 2; index++){
                FFTMag[index] = (Uint16)abs(IOBuffer2[index]);
            }
            Debug++;

            // Clear Buffers
            for(index = 0; index < 2*NSAMPLES + 2  ; index++)
            {
                IOBuffer2[index] = 0.0;
                FFTMag[index] = 0;
            }
            for(index = 0; index < 2*NSAMPLES  ; index++)
            {
                IOBuffer[index] = 0.0;
            }

//            asm(" ESTOP0"); // forces the program to stop so that the magnitude buffer is not changed
        }
    }

}
// End of main

//*****************************************************************************
// ISR
//*****************************************************************************

interrupt void ADC_ISR(void){

//    ADCBuff[ADCCnt] = (AdcaResultRegs.ADCRESULT0);
//    IOBuffer[ADCCnt] = ((float)ADCBuff[ADCCnt])*(3.3/4096.0) - 1.65;
//    ADCCnt++;
//
//    if(ADCCnt == 2*NSAMPLES - 1){
//        ADCCnt = 0;
//        if(FFTflag == 0){
//            DINT; // stops any more ADC samples to be taken
//            FFTflag = 1;
//        }
//    }

    GpioDataRegs.GPATOGGLE.bit.GPIO19 = 1; // each time a sample is taken, the Pin toggles (pin 3 on launchpad)

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear ADC interrupt flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void cla1Isr1 ()
{
    // Acknowledge the end-of-task interrupt for task 1
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
//  asm(" ESTOP0");
}


__interrupt void cla1Isr2 ()
{
    asm(" ESTOP0");

}
// @} //addtogroup

// End of file
