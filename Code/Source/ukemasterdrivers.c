#include "ukemasterdrivers.h"

#if CPU_FRQ_200MHZ
#define SPI_BRR        ((200E6 / 4) / 500E3) - 1
#endif

#if CPU_FRQ_150MHZ
#define SPI_BRR        ((150E6 / 4) / 500E3) - 1
#endif

#if CPU_FRQ_120MHZ
#define SPI_BRR        ((120E6 / 4) / 500E3) - 1
#endif

// Push Button ISR Definitions
interrupt void Play_ISR(void);
interrupt void Record_ISR(void);
interrupt void Prev_ISR(void);
interrupt void Next_ISR(void);

// ADC ISR definition
interrupt void Tempo_ISR(void);
interrupt void ADC_ISR(void);
interrupt void FFTSync_ISR(void);

void SpiTransmit_FPGA(uint16_t data)
{
    /* Transmit 16 bit data */
    SpiaRegs.SPIDAT = data; //send data to SPI register _IOBuffer
    while (SpiaRegs.SPISTS.bit.INT_FLAG == 0); //wait until the data has been sent
    Uint16 dummyLoad = SpiaRegs.SPIRXBUF; //reset flag
}

void InitSPIA(void)
{
    /* Init GPIO pins for SPIA */

    //enable pullups for each pin
    //set to asynch qualification
    //configure each mux
    // SPI A SS -> GPIO19
    //SPI A SIMO -> GPIO58
    // SPI B SIMO -> GPIO63
    //SPI A CLK -> GPIO18
    // SPI B CLK -> GPIO65
    EALLOW;

    //enable pullups
//    GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;
//    GpioCtrlRegs.GPCPUD.bit.GPIO65 = 0;
//    GpioCtrlRegs.GPBPUD.bit.GPIO63 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO58 = 0;

//    GpioCtrlRegs.GPAGMUX2.bit.GPIO19 = 0;
    GpioCtrlRegs.GPAGMUX2.bit.GPIO18 = 0;
    GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = 3;

//    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 1;
    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1;
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 3;

    //asynch qual
//    GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3;
    GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3;
    GpioCtrlRegs.GPBQSEL2.bit.GPIO58 = 3;

    EDIS;

    /* Init SPI peripheral */
    SpiaRegs.SPICCR.all = 0x4B; //CLKPOL = 0, SOMI = SIMO (loopback), 12 bit characters
    SpiaRegs.SPICTL.all = 0x06; //master mode, enable transmissions
    SpiaRegs.SPIBRR.all = SPI_BRR; //gives baud rate of approx 850 kHz LSPCLK

    SpiaRegs.SPICCR.bit.SPISWRESET = 1;
    SpiaRegs.SPIPRI.bit.FREE = 1;

    //now the pins
    //rst
    GPIO_SetupPinOptions(67, 1, 0);
    GPIO_WritePin(67, 1);
    //chip select
    GPIO_SetupPinOptions(19, 1, 0);

}

void SpiTransmit_LEDs(uint8_t data)
{
    /* Transmit 16 bit data */
    SpibRegs.SPIDAT = data << 8; //send data to SPI register _IOBuffer
    while (SpibRegs.SPISTS.bit.INT_FLAG == 0); //wait until the data has been sent
    Uint16 dummyLoad = SpibRegs.SPIRXBUF; //reset flag
}

void InitSPIB(void)
{
    EALLOW;

    // configure GPIO for SPI
    GpioCtrlRegs.GPBMUX2.all  = 0xC0000000;
    GpioCtrlRegs.GPBGMUX2.all = 0xC0000000;
    GpioCtrlRegs.GPCMUX1.all  = 0x0000000F; // configures SPI on pins 63,64, and 65
    GpioCtrlRegs.GPCGMUX1.all = 0x0000000F;
    GpioCtrlRegs.GPBQSEL2.all = 0xC0000000;
    GpioCtrlRegs.GPCQSEL1.all = 0x03;
    GpioDataRegs.GPCSET.all   = 0x0000000C; // only the chip selects are outputs and they are initially high
    GpioCtrlRegs.GPCDIR.all   = 0x0000000C;

    // Disable SPI-B and reset its registers
    SpibRegs.SPICCR.bit.SPISWRESET = 0;

    // Set up SPI-B clock
    SpibRegs.SPICCR.bit.CLKPOLARITY = 0; // CPOL = 0
    SpibRegs.SPICCR.bit.SPICHAR = 7; // 8-bit data
    SpibRegs.SPICCR.bit.SPILBK = 1; // No loopback mode
    SpibRegs.SPIBRR.all = 89; // Baud rate = SYSCLKOUT / (SPIBRR+1) = 200MHz / (63+1) = 3.125MHz

    // Set up SPI-B control
    SpibRegs.SPICTL.bit.TALK = 1; // Enable transmission
    SpibRegs.SPICTL.bit.MASTER_SLAVE = 1; // Master mode
    SpibRegs.SPICTL.bit.CLK_PHASE = 1; // CPHA = 1
    SpibRegs.SPICTL.bit.SPIINTENA = 0; // Disable SPI interrupts

    // Enable SPI-B
    SpibRegs.SPICCR.bit.SPISWRESET = 1;
    SpibRegs.SPIPRI.bit.FREE = 1; // Set SPI-B to low priority
    SpibRegs.SPIPRI.bit.SOFT = 1; // Set SPI-B to software-controlled mode

}

void InitTimer0(void) { // Play Tempo Clock
    EALLOW;
    PieVectTable.TIMER0_INT = &Tempo_ISR;
    IER |= M_INT1;
    ConfigCpuTimer(&CpuTimer0, 200, 100);   // Configure CPU timer 1. 200 -> SYSCLK in MHz, 100us -> period in usec. NOTE: Does NOT start timer
    CpuTimer0.RegsAddr->TCR.bit.TSS = 0;        // Start timer 1
}

void InitTimer1(void) { // ADC CLK
    ConfigCpuTimer(&CpuTimer1, 200, 98);   // Configure CPU timer 1. 200 -> SYSCLK in MHz, 100ms -> period in usec. NOTE: Does NOT start timer

}

void InitTimer2(void) { // FFT sync clock
    EALLOW;
    PieVectTable.TIMER2_INT = &FFTSync_ISR;
    IER |= M_INT14;
    ConfigCpuTimer(&CpuTimer2, 200, 101000);   // Configure CPU timer 1. 200 -> SYSCLK in MHz, 100ms -> period in usec. NOTE: Does NOT start timer

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

void InitEPwm1(void){
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // Set GPIO0 as PWM output
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;  // Set GPIO0 as output
    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1; // Set initial output to low
    EDIS;

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_FREEZE; // Freeze counter
    EPwm1Regs.TBPRD = 90; // Set period to 40 (40 MHz / 40 = 1 MHz)
    EPwm1Regs.CMPA.bit.CMPA = 71; // Set duty cycle to 30% (12 / 40 = 30%)
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET; // Set output to high on counter zero
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR; // Set output to low on compare match
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up-down mode
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE; // Disable phase loading
    EPwm1Regs.TBPHS.bit.TBPHS = 0; // Phase is 0
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1; // HSPCLK = SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1; // TBCLK = HSPCLK
}

/*
 * Summary: Configures GPIO Pins to accommodate UI of UkeMaster 3000
 */
void GPIOInit(void){

    // allow registers to be modified
    EALLOW;

    // Set LED Bar Connections as outputs
    GpioCtrlRegs.GPADIR.bit.GPIO29 = 1;
    GpioCtrlRegs.GPDDIR.bit.GPIO125 = 1;
    GpioCtrlRegs.GPDDIR.bit.GPIO124 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO59 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;
    GpioCtrlRegs.GPDDIR.bit.GPIO122 = 1;
    GpioCtrlRegs.GPDDIR.bit.GPIO123 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO61 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;

    GpioDataRegs.GPASET.bit.GPIO29 = 1; // Turn off LEDs
    GpioDataRegs.GPDSET.bit.GPIO125 = 1;
    GpioDataRegs.GPDSET.bit.GPIO124 = 1;
    GpioDataRegs.GPBSET.bit.GPIO59 = 1;
    GpioDataRegs.GPASET.bit.GPIO5 = 1;
    GpioDataRegs.GPASET.bit.GPIO4 = 1;
    GpioDataRegs.GPDSET.bit.GPIO122 = 1;
    GpioDataRegs.GPDSET.bit.GPIO123 = 1;
    GpioDataRegs.GPBSET.bit.GPIO61 = 1;
    GpioDataRegs.GPASET.bit.GPIO1 = 1;

    // Configure LED Strip Data as an output
    GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;

    // Configure Passthrough Pins
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 0;
    GpioCtrlRegs.GPDDIR.bit.GPIO97 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO94 = 0;
    // ADCB5 and A5 as well

    // Configure Pushbuttons as inputs
    GpioCtrlRegs.GPEDIR.bit.GPIO130 = 0;
    GpioCtrlRegs.GPEDIR.bit.GPIO131 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO66 = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 0;

    // Configure External interrupts for push buttons
    GPIO_SetupXINT1Gpio(130); //Play
    GPIO_SetupXINT2Gpio(131); //record
    GPIO_SetupXINT3Gpio(66); //prev
    GPIO_SetupXINT4Gpio(6); //next

    EALLOW;
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1; // Play
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1; // Record
    PieCtrlRegs.PIEIER12.bit.INTx1 = 1; // Next Track
    PieCtrlRegs.PIEIER12.bit.INTx2 = 1; // Previous Track

    XintRegs.XINT1CR.bit.POLARITY = 1; // rising edge triggered
    XintRegs.XINT2CR.bit.POLARITY = 3; // either edge triggered
    XintRegs.XINT3CR.bit.POLARITY = 1; // rising edge triggered
    XintRegs.XINT4CR.bit.POLARITY = 1; // rising edge triggered

    // enable all
    XintRegs.XINT1CR.bit.ENABLE = 1;
    XintRegs.XINT2CR.bit.ENABLE = 1;
    XintRegs.XINT3CR.bit.ENABLE = 1;
    XintRegs.XINT4CR.bit.ENABLE = 1;

    PieVectTable.XINT1_INT = &Play_ISR;
    PieVectTable.XINT2_INT = &Record_ISR;
    PieVectTable.XINT3_INT = &Next_ISR;
    PieVectTable.XINT4_INT = &Prev_ISR;

    IER |= M_INT1;
    IER |= M_INT12;
}


