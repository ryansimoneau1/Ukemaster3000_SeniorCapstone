#include <F28x_Project.h>
#include "F2837xD_device.h"
#include "device.h"
#include <stdio.h>

#ifndef UKEMASTERDRIVERS_H_
#define UKEMASTERDRIVERS_H_

//spi A
void InitSPIA(void);
void SpiTransmit_FPGA(uint16_t data);

void SpiTransmit_LEDs(uint8_t data);
void InitSPIB(void);

/*
 * Summary: Sets the rate the ADC takes samples. should be 100ms/1024 = 97us
 */
void InitTimer0(void);

/*
 * Summary: Controls the rate that FFTs are taken on ADC samples in IOBuffer
 */
void InitTimer1(void);

/*
 * Summary: controls tempo of songs (rate that DSP sends notes to FPGA
 */
void InitTimer2(void);

/*
 * Summary: Configure ADC to sample 12 bit single ended
 */
void ConfigureADC(void);

void InitEPwm1(void);

/*
 * Summary: Configures GPIO Pins to accommodate UI of UkeMaster 3000
 */
void GPIOInit(void);

#endif /* UKEMASTERDRIVERS_H_ */
