#include <F28x_Project.h>
#include "F2837xD_device.h"
#include "device.h"
#include <stdio.h>
#include <stdlib.h>
#include "OneToOneI2CDriver.h"

#ifndef UKEMASTERFUNC_H_
#define UKEMASTERFUNC_H_

// LED Block defines
#define OffLED1 GpioDataRegs.GPASET.bit.GPIO29 = 1 // LEDs turn on when Output is low
#define OffLED2 GpioDataRegs.GPDSET.bit.GPIO125 = 1
#define OffLED3 GpioDataRegs.GPDSET.bit.GPIO124 = 1
#define OffLED4 GpioDataRegs.GPBSET.bit.GPIO59 = 1
#define OffLED5 GpioDataRegs.GPASET.bit.GPIO5 = 1
#define OffLED6 GpioDataRegs.GPASET.bit.GPIO4 = 1
#define OffLED7 GpioDataRegs.GPDSET.bit.GPIO122 = 1
#define OffLED8 GpioDataRegs.GPDSET.bit.GPIO123 = 1
#define OffLED9 GpioDataRegs.GPBSET.bit.GPIO61 = 1
#define OffLED10 GpioDataRegs.GPASET.bit.GPIO1 = 1

#define OnLED1 GpioDataRegs.GPACLEAR.bit.GPIO29 = 1
#define OnLED2 GpioDataRegs.GPDCLEAR.bit.GPIO125 = 1
#define OnLED3 GpioDataRegs.GPDCLEAR.bit.GPIO124 = 1
#define OnLED4 GpioDataRegs.GPBCLEAR.bit.GPIO59 = 1
#define OnLED5 GpioDataRegs.GPACLEAR.bit.GPIO5 = 1
#define OnLED6 GpioDataRegs.GPACLEAR.bit.GPIO4 = 1
#define OnLED7 GpioDataRegs.GPDCLEAR.bit.GPIO122 = 1
#define OnLED8 GpioDataRegs.GPDCLEAR.bit.GPIO123 = 1
#define OnLED9 GpioDataRegs.GPBCLEAR.bit.GPIO61 = 1
#define OnLED10 GpioDataRegs.GPACLEAR.bit.GPIO1 = 1

// LED Block Magnitude boundaries
#define LEDBoundary1 2453
#define LEDBoundary2 2570
#define LEDBoundary3 2662
#define LEDBoundary4 2867
#define LEDBoundary5 3072
#define LEDBoundary6 3277
#define LEDBoundary7 3482
#define LEDBoundary8 3686
#define LEDBoundary9 3891
#define LEDBoundary10 4096

// LED Strip
#define LEDSize 144
#define ZeroBin 60 // magnitudes under this will not show up on the spectrogram
#define ColorBrightness 255 // max brightness of each R G B color in form of 8-bit hex code
#define MagBin 25 // size of the color bins for the spectrogram
#define ColorSlope (ColorBrightness / (ZeroBin + 5*MagBin)) // determines how colors add together in each magnitude bin

// note bin definitions -- the bin is the upper bound for each note
#define B3  51
#define C4  54
#define Db4 57
#define D4  60
#define Eb4 64
#define E4  68
#define F4  72
#define Gb4 76
#define G4  81
#define Ab4 86
#define A4  91
#define Bb4 96
#define B4  102
#define C5  108
#define Db5 114
#define D5  121

// FFT Processing
#define NumPeaks 4
#define MinPeakValue 50

/*
 * Summary: Takes input of peaks and their bins/frequencies. From there, assigns each note to a string
 *
 * Note: If a note is in the range of a string that already has a note, it checks if it can
 *       make it onto the next string. If not, it isn't mapped to the fretboard
 *
 * bins: input of array of bins of the peaks, sorted in descending order of peak magnitude
 * data: Uint16 data which represents the bits to be sent to send_notes function
 *
 * data bits: 0b  XXX       XXX        XXX        XXX      0000
 *               String 1 | String 2 | String 3 | String 4
 *
 */
Uint16 SortNotes(Uint16 *bins);

/*
 * Summary: Sends SPI data to the FPGA given an input array of data and its size
 *
 * sdata: The input array, each element in the array is a string of bits to be sent to FPGA
 * sdelay: The delay between each element in sdata. In other words, time before each new
 *          note/chord is played on the uke
 * size: size of both arrays (should be the same)
 */
void SendNotes(Uint16 *sdata, Uint16 size);

/*
 * Summary: Finds a number of peaks in the FFT as defined by the user. Additionally allows user to configure the number of
 *          bins away from a found peak to start searching for the next peak. Sorts all peaks in descending order in the output buffer
 *
 * FFTmagnitude: The Raw FFT magnitude data
 * StartBin: Start of interval to look for peaks in
 * StopBin: End of interval to look for peaks in
 * NumPeaks: Number of peaks to find
 * PeakWidth: Number of bins away from a found peak that another peak can be found
 */
void FindPeaks(Uint16 *FFTmagnitude, Uint16 StartBin, Uint16 StopBin, Uint16 PeakWidth, Uint16 *PeakMags, Uint16 *PeakBins);

/*
 * Summary: Takes FFT values and decimates them to the number of LEDs available in strip. Allows for range of frequencies to be
 *          based on target # of Bins per LED and LED strip Size.
 *
 * FFTmag: FFT Magnitude data computed by CLA
 * StartBin: Starting frequency for output
 * BinsPerLED: Target # of bins per LED
 * LEDdata: Pass by reference output values
 */
void DecimateFFT(Uint16 *FFTmag, Uint16 StartBin, Uint16 BinsPerLED, Uint16 *LEDdata);

void Spectrum(Uint16 *FFTVals, Uint16 Size, uint8_t *r, uint8_t *g, uint8_t *b);

void MyLEDArray(Uint16 *r, Uint16 *g, Uint16 *b);

void shift_array(Uint16 *arr);

/*
 * Summary: Finds Max value in microphone signal and lights up LEDs on LED bar to signify how close the input is to the ADC rails.
 *          Size corresponds to the number of samples in ADCSamples that are used to find the max value of the input signal. Larger
 *          size will improve the accuracy of the function, but the process will take longer.
 *
 * ADCSamples: Microphone ADC Buffer holding audio values
 * Size: Amount of buffer used to find the max value
 */
void LEDBar(Uint16 *ADCSamples, Uint16 Size, Uint16 LowLimit);

/*
 * Summary: Function decides what is displayed on the LCD based on what track is selected and wether the user wants to play or
 *          record a song
 *
 * TrackNumber: The track being played or recorded
 * PlayFlag: means that the song is going to be played
 * RecordFlag: means the song is being recorded
 */
void UpdateLCDFrame(Uint16 TrackNumber, Uint16 PlayFlag, Uint16 RecordFlag);
// send single commands to the LCD
void LCDCTRL(Uint16 command);

// Function to send single characters to the LCD
void LCDDATA(Uint16 data);

// function with hardcodded values meant to initialize the LCD
void LCDINIT(void);

// function to send string to the LCD
void LCDSTRING(char * const string, Uint16 length);

// initializes the SPI system
void SPIINIT(void);

// send data to the SRAMs via SPIA
Uint16 SPITRANSMIT(Uint16 data);

#endif /* UKEMASTERFUNC_H_ */
