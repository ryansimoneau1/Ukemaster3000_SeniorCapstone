#include <F28x_Project.h>
#include <float.h>
#include <stdio.h>
#include "f2837x_fft_examples_setup.h"
#include "F2837xD_device.h"
#include "rfft_1024_shared.h"
#include "OneToOneI2CDriver.h"
#include "ukemasterfunc.h"
#include "ukemasterdrivers.h"
#include "board.h"
#include "clb_config.h"
#include "driverlib.h"
#include "device.h"

// FFT
#define NSTAGES         9               // 512
#define NSAMPLES        (1<<NSTAGES)
#define TRANSFER        1024
#define BURST           1

// Program Control
#define TrackLength 300     // total number of notes to send to the FPGA (100 corresponds with 10 seconds or 1000 FFTs)
#define NumTracks 5         // The total number of tracks

//*****************************************************************************
// globals
//*****************************************************************************

// FFT variables
#pragma DATA_SECTION(IOBuffer,"IOBuffer")
float IOBuffer[2*NSAMPLES];
#pragma DATA_SECTION(IOBuffer2,"IOBuffer")
float IOBuffer2[2*NSAMPLES + 2];
Uint16 FFTMag[2*NSAMPLES];
Uint16 ADCBuff[2*NSAMPLES];
Uint16 IOBufferCnt      = 0; // ADC sample #
Uint16 FFTComputeFlag   = 0; // Flag that indicates FFT completion
Uint16 FFTDoneFlag      = 0; // Signals that the FFT is done

// FFT Processing
Uint16 PeakMags[NumPeaks]; // stores the peaks and their magnitudes for the 4 notes to play in a window. Stored in descending order of magnitude
Uint16 PeakBins[NumPeaks];
Uint16 PrevPeakMags[NumPeaks];
Uint16 SameFlag = 0;
Uint16 LEDdata[LEDSize];

enum Uicommand {
    Record      = 1,
    Play        = 2,
    Prev        = 3,
    Next        = 4,
    Idle        = 5
};

enum Uicommand Task = Idle;

// Task Flags
Uint16 PlayFlag         = 0;
Uint16 RecordFlag       = 0;
Uint16 UpdateLCDFlag    = 1;

// Track Buffers
Uint16 TrackNum         = 0; // Track to record or play
Uint16 FrameCnt         = 0; // Which individual FFT is being processed and added to Track#
Uint16 BufferFullFlag   = 0; // indicates that the track buffer is full and an FFT can be performed
Uint16 Track0[TrackLength];
Uint16 TrackDebug[TrackLength];
Uint16 Track1[TrackLength];
Uint16 Track2[TrackLength];

//mary had a little lamb track
Uint16 Track3[141];
Uint16 TrueTrackLength[NumTracks] = {0,0,0};
Uint16 temp = 0;

// LED Strip Variables
Uint16 LEDCnt = 0; // keeps track of what LEDs recieve data
Uint16 ColorCnt = 0; // keeps track of what color to send
Uint16 Decimate[LEDSize];
Uint16 Zeros[LEDSize] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
Uint16 r[LEDSize];
Uint16 g[LEDSize];
Uint16 b[LEDSize];

// LEDDemo Variables
Uint16 rDEMO[LEDSize] = {128,154,179,202,222,238,249,254,254,249, // data from sine lookup tables at various frequencies
                     238,222,202,179,154,128,101,76,53,33,
                     17,6,1,1,6,17,33,53,76,101,128,154,179,202,222,238,249,254,254,249,
                     238,222,202,179,154,128,101,76,53,33,
                     17,6,1,1,6,17,33,53,76,101,128,154,179,202,222,238,249,254,254,249,
                     238,222,202,179,154,128,101,76,53,33,
                     17,6,1,1,6,17,33,53,76,101,128,154,179,202,222,238,249,254,254,249,
                     238,222,202,179,154,128,101,76,53,33,
                     17,6,1,1,6,17,33,53,76,101,238,222,202,179,154,128,101,76,53,33,
                     17,6,1,1};
Uint16 gDEMO[LEDSize] = {124,127,127,124,119,111,101,89,77,64,
                     50,38,26,16,8,3,0,0,3,8,16,26,38,50,
                     64,77,89,101,111,119,124,127,127,124,119,111,101,89,77,64,
                     50,38,26,16,8,3,0,0,3,8,16,26,38,50,
                     64,77,89,101,111,119,124,127,127,124,119,111,101,89,77,64,
                     50,38,26,16,8,3,0,0,3,8,16,26,38,50,
                     64,77,89,101,111,119,124,127,127,124,119,111,101,89,77,64,
                     50,38,26,16,8,3,0,0,3,8,16,26,38,50,
                     64,77,89,101,111,119,124,127,127,124,119,111,101,89,77,64,
                     50,38,26,16,8,3,0,0,3,8,16,26,38,50};
Uint16 bDEMO[LEDSize] = {128,179,222,249,254,238,202,154,101,
                     53,17,1,6,33,76,128,179,222,249,254,
                     238,202,154,101,53,17,1,6,33,76,128,179,222,249,254,238,202,154,101,
                     53,17,1,6,33,76,128,179,222,249,254,
                     238,202,154,101,53,17,1,6,33,76,128,179,222,249,254,238,202,154,101,
                     53,17,1,6,33,76,128,179,222,249,254,
                     238,202,154,101,53,17,1,6,33,76,128,179,222,249,254,238,202,154,101,
                     53,17,1,6,33,76,128,179,222,249,254,
                     238,202,154,101,53,17,1,6,33,76,128,179,222,249,254,238,202,154,101,
                     53,17,1,6,33,76,128,179,222,249,254,
                     238,202,154,101};
Uint16 LEDdelay = 0;
Uint16 LEDdelay2 = 0;
Uint16 DelayDone = 0;
int main(void){

    // Clear out the final result buffer
    for(Uint16 i = 0; i < 2*NSAMPLES + 2  ; i++)
    {
        IOBuffer2[i] = 0.0;
    }

    for (Uint16 i = 0; i < 141; i++) {
        Track3[i] = 0b1111111111110000;
    }
    Track3[5] = 0b1111110011110000;
    Track3[10] = 0b1110111111110000;
    Track3[15] = 0b1110011111110000;
    Track3[20] = 0b1110111111110000;
    Track3[25] = 0b1111110011110000;
    Track3[30] = 0b1111110011110000;
    Track3[35] = 0b1111110011110000;
    Track3[45] = 0b1110111111110000;
    Track3[50] = 0b1110111111110000;
    Track3[55] = 0b1110111111110000;
    Track3[65] = 0b1111110011110000;
    Track3[70] = 0b1111111001110000;
    Track3[75] = 0b1111111001110000;
    Track3[80] = 0b1111110011110000;
    Track3[85] = 0b1110111111110000;
    Track3[90] = 0b1110011111110000;
    Track3[95] = 0b1110111111110000;
    Track3[100] = 0b1111110011110000;
    Track3[105] = 0b1111110011110000;
    Track3[110] = 0b1111110011110000;
    Track3[115] = 0b1111110011110000;
    Track3[120] = 0b1110111111110000;
    Track3[125] = 0b1110111111110000;
    Track3[130] = 0b1111110011110000;
    Track3[135] = 0b1110111111110000;
    Track3[140] = 0b1110011111110000;

//    InitSysCtrl();
    CLA_DSP_initSystemClocks();
    CLA_DSP_configClaMemory();
    CLA_DSP_initCpu1Cla1();

    EALLOW;
    InitSPIB();
    InitEPwm1();

    EALLOW; // allows ADC Registers to be modified

//    GpioCtrlRegs.GPADIR.bit.GPIO19 = 1; // Set pin as output (pin 3 on launchpad)
    I2C_O2O_Master_Init(0x27, 200.0, 12);
    LCDINIT();
    LCDCTRL(0x0C); // disable cursor blinking, shut off the cursor

    DINT;               // Disable CPU interrupts on startup

    // Init PIE
    InitPieCtrl();      // Initialize PIE -> disable PIE and clear all PIE registers
    IER = 0x0000;       // Clear CPU interrupt register
    IFR = 0x0000;       // Clear CPU interrupt flag register
    InitPieVectTable(); // Initialize PIE vector table to known state


    EALLOW;
    InitSPIA();
    Board_init();
    initTILE1(CLB1_BASE);
    CLB_enableCLB(CLB1_BASE);

    GPIOInit();
    InitCpuTimers();    // Initialize all timers to known state
    InitTimer1();
    InitTimer2();
    CpuTimer1.RegsAddr->TCR.bit.TSS = 0;        // Start ADC Timer //start timers for Idle Spectrogram display
    CpuTimer2.RegsAddr->TCR.bit.TSS = 0;        // Start FFT Timer
    ConfigureADC();

    EnableInterrupts(); // Enable PIE and CPU interrupts. Includes EINT

//    EDIS; // disable Editing of control registers


    UpdateLCDFlag = 1; // Update LCD at start to show initial Frame

    while(1){
        switch(Task){ // control flow of Ukemaster
        case Record:
            if(UpdateLCDFlag){
                DINT; // Disable interrupts when LCD is receiving commands
                UpdateLCDFrame(TrackNum, PlayFlag, RecordFlag);
                UpdateLCDFlag = 0; // LCD has been updated and Record/play/track change can happen
                EINT; //re-enable interrupts after LCD has been updated
            }
            if(!BufferFullFlag){ // Only start timers if the buffer hasn't been filled already
                CpuTimer1.RegsAddr->TCR.bit.TSS = 0;        // Start ADC Timer
                CpuTimer2.RegsAddr->TCR.bit.TSS = 0;        // Start FFT Sync Timer
            }
            FrameCnt = 0;
            while(Task == Record && !BufferFullFlag && GPIO_ReadPin(131) == 1 && FrameCnt < TrackLength && TrackNum != 4){
                if(FFTComputeFlag == 1){ // Last resort for if CLA interrupts aren't working
                    FFTComputeFlag = 0;
                    while (CLA_getTaskRunStatus(CLA1_BASE, CLA_TASK_1) == 1); // wait for CLA to compute the FFT
                    FFTDoneFlag = 1;
                }

                if(FFTDoneFlag == 1){
                    FFTDoneFlag = 0;
                    // Restart the ADC
                    for(Uint16 i = 1; i < 2*NSAMPLES; i++){ // i start at 1 to remove DC component
                        FFTMag[i] = (Uint16)abs(IOBuffer2[i]);
                    }
                    asm(" NOP");
                    for(Uint16 i = 0; i < 2*NSAMPLES + 2  ; i++) // clear input and output buffers
                    {
                        IOBuffer[i] = 0.0;
                        IOBuffer2[i] = 0.0;
//                        FFTMag[i] = 0; // For Debugging purposes only
                    }
                    CpuTimer1.RegsAddr->TCR.bit.TSS = 0;        // Start ADC Timer
                    //## FFT PROCESSING ##//
                    //decimating for LED
                    DecimateFFT(FFTMag, 40, 2,Decimate);
                    Spectrum(Decimate, LEDSize, r, g, b);
                    MyLEDArray(r, g, b);

                    //finding peaks
                    FindPeaks(FFTMag, 51, 122, 15,PeakMags,PeakBins); // 50: start at 250 Hz, 122: end at 610Hz, 4: allow peaks to have 20 Hz worth of spread

                    //placing notes into track arrays
                    if (TrackNum == 0) {
                        //making peak data readable to the FPGA
                        temp = SortNotes(PeakBins);
                        TrackDebug[FrameCnt] = temp;
                        //checking if previous notes were the same, if they were, act as if no new note
                        if (FrameCnt > 0 && (Track0[FrameCnt-1] == temp || SameFlag == temp)) {
                            Track0[FrameCnt] = 0b1111111111110000;
                            SameFlag = temp;
                        } else {
                            //to reduce spastic behavior, check if there's been a new note in the last 2 notes
                            //if not, play this one, if so, don't play
                            //this makes it so there's at most one new note every 3 notes
                            if (FrameCnt > 3 && Track0[FrameCnt-4] != 0b1111111111110000) {
                                Track0[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 2 && Track0[FrameCnt-3] != 0b1111111111110000) {
                                Track0[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 1 && Track0[FrameCnt-2] != 0b1111111111110000) {
                                Track0[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 0 && Track0[FrameCnt-1] != 0b1111111111110000) {
                                Track0[FrameCnt] = 0b1111111111110000;
                            } else {
                                Track0[FrameCnt] = temp;
                            }
                            SameFlag = 0b1111111111110000;
                        }
                        TrueTrackLength[0] = FrameCnt;
                    } else if (TrackNum == 1) {
                        //making peak data readable to the FPGA
                        temp = SortNotes(PeakBins);
                        TrackDebug[FrameCnt] = temp;
                        //checking if previous notes were the same, if they were, act as if no new note
                        if (FrameCnt > 0 && (Track1[FrameCnt-1] == temp || SameFlag == temp)) {
                            Track1[FrameCnt] = 0b1111111111110000;
                            SameFlag = temp;
                        } else {
                            //to reduce spastic behavior, check if there's been a new note in the last 2 notes
                            //if not, play this one, if so, don't play
                            //this makes it so there's at most one new note every 3 notes
                            if (FrameCnt > 3 && Track1[FrameCnt-4] != 0b1111111111110000) {
                                Track1[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 2 && Track1[FrameCnt-3] != 0b1111111111110000) {
                                Track1[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 1 && Track1[FrameCnt-2] != 0b1111111111110000) {
                                Track1[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 0 && Track1[FrameCnt-1] != 0b1111111111110000) {
                                Track1[FrameCnt] = 0b1111111111110000;
                            } else {
                                Track1[FrameCnt] = temp;
                            }
                            SameFlag = 0b1111111111110000;
                        }
                        TrueTrackLength[1] = FrameCnt;
                    } else {
                        //making peak data readable to the FPGA
                        temp = SortNotes(PeakBins);
                        TrackDebug[FrameCnt] = temp;
                        //checking if previous notes were the same, if they were, act as if no new note
                        if (FrameCnt > 0 && (Track2[FrameCnt-1] == temp || SameFlag == temp)) {
                            Track2[FrameCnt] = 0b1111111111110000;
                            SameFlag = temp;
                        } else {
                            //to reduce spastic behavior, check if there's been a new note in the last 2 notes
                            //if not, play this one, if so, don't play
                            //this makes it so there's at most one new note every 3 notes
                            if (FrameCnt > 3 && Track2[FrameCnt-4] != 0b1111111111110000) {
                                Track2[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 2 && Track2[FrameCnt-3] != 0b1111111111110000) {
                                Track2[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 1 && Track2[FrameCnt-2] != 0b1111111111110000) {
                                Track2[FrameCnt] = 0b1111111111110000;
                            } else if (FrameCnt > 0 && Track2[FrameCnt-1] != 0b1111111111110000) {
                                Track2[FrameCnt] = 0b1111111111110000;
                            } else {
                                Track2[FrameCnt] = temp;
                            }
                            SameFlag = 0b1111111111110000;
                        }
                        TrueTrackLength[2] = FrameCnt;
                    }

                    // Possibly remove some frames from the current track if the previous
                    // frame is the same, but the current frame has lower peaks

                    // Make sure to clear LEDdata, PeakMags, and PeakBins before the next cycle //
                    for (Uint16 i = 0; i < NumPeaks; i++) {
                        PrevPeakMags[i] = PeakMags[i];
                        PeakBins[i] = 0;
                        PeakMags[i] = 0;
                    }

                    //####################//
                    FrameCnt++; // post increment frame # after each FFT is computed
                    if(FrameCnt >= (TrackLength - 1)){
                        FrameCnt = 0;
                        BufferFullFlag = 1;
                        CpuTimer1.RegsAddr->TCR.bit.TSS = 1;        // Stop ADC Timer
                        CpuTimer2.RegsAddr->TCR.bit.TSS = 1;        // Stop FFT Sync Timer
                    }
                }
            }
            Task = Idle;
            CpuTimer1.RegsAddr->TCR.bit.TSS = 1;        // Stop ADC Timer
            CpuTimer2.RegsAddr->TCR.bit.TSS = 1;        // Stop FFT Sync Timer
            RecordFlag      = 0; // indicate Recording has stopped on LCD
            BufferFullFlag  = 0; // Allow future recordings to occur
            FFTComputeFlag  = 0; // Has to wait to do FFT in next record session
            FFTDoneFlag     = 0; // Has to wait until new FFT is computed to process FFTMag
            IOBufferCnt     = 0; // reset IOBuffer pointer to zero
    //        FrameCnt        = 0; // Record Track notes from the begining

            for(Uint16 i = 0; i < 2*NSAMPLES + 2  ; i++) // clear input and output buffers
            {
                IOBuffer[i] = 0.0;
                IOBuffer2[i] = 0.0;
                FFTMag[i] = 0;
            }

            UpdateLCDFlag = 1;
            break;
        case Play:
            if(UpdateLCDFlag){
                DINT; // Disable interrupts when LCD is receiving commands
                UpdateLCDFrame(TrackNum, PlayFlag, RecordFlag);
                UpdateLCDFlag = 0; // LCD has been updated and Record/play/track change can happen
                CpuTimer1.RegsAddr->TCR.bit.TSS = 1;    // Stop ADC
                MyLEDArray(Zeros, Zeros, Zeros);
                OffLED1;
                OffLED2;
                OffLED3;
                OffLED4;
                OffLED5;
                OffLED6;
                OffLED7;
                OffLED8;
                OffLED9;
                OffLED10;
                EINT; //re-enable interrupts after LCD has been updated
            }

            if(TrackNum == 4){
                for(Uint16 i = 0; i < 12; i++){
                    for(Uint16 j = 0; j < 15; j++){
                        MyLEDArray(rDEMO,gDEMO,bDEMO); // Turn Half the strip Red
                        shift_array(rDEMO);
                        shift_array(gDEMO);
                        shift_array(gDEMO);
                        shift_array(bDEMO);
                        DELAY_US(50000); // Reset time for debugging and such
                    }
                }
            }

            //choosing track to play
            if (TrackNum == 0)  {
                SendNotes(Track0,TrueTrackLength[0]);
            } else if (TrackNum == 1) {
                SendNotes(Track1,TrueTrackLength[1]);
            } else if (TrackNum == 2) {
                SendNotes(Track2,TrueTrackLength[2]);
            } else if (TrackNum == 3) {
                SendNotes(Track3,141);
            }

            PlayFlag = 0;
            CpuTimer1.RegsAddr->TCR.bit.TSS = 0;    // Start ADC Timer
            UpdateLCDFlag = 1;
            Task = Idle;
            break;
        case Prev:
            if(UpdateLCDFlag){
                DINT; // Disable interrupts when LCD is receiving commands
                UpdateLCDFrame(TrackNum, PlayFlag, RecordFlag);
                UpdateLCDFlag = 0; // LCD has been updated and Record/play/track change can happen
                EINT; //re-enable interrupts after LCD has been updated
            }
            if(TrackNum == 0){    // go to the last track if decrement past 0
                TrackNum = (NumTracks - 1);
            }else{
                TrackNum--;
            }
            UpdateLCDFlag = 1;
            Task = Idle;
            break;
        case Next:
            if(UpdateLCDFlag){
                DINT; // Disable interrupts when LCD is receiving commands
                UpdateLCDFrame(TrackNum, PlayFlag, RecordFlag);
                UpdateLCDFlag = 0; // LCD has been updated and Record/play/track change can happen
                EINT; //re-enable interrupts after LCD has been updated
            }
            if(TrackNum == (NumTracks - 1)){    // go to the first track if increment past the # of tracks
                TrackNum = 0;
            }else{
                TrackNum++;
            }
            UpdateLCDFlag = 1;
            Task = Idle;
            break;
        case Idle:
            if(UpdateLCDFlag){
                DINT; // Disable interrupts when LCD is receiving commands
                UpdateLCDFrame(TrackNum, PlayFlag, RecordFlag);
                UpdateLCDFlag = 0; // LCD has been updated and Record/play/track change can happen
                CpuTimer1.RegsAddr->TCR.bit.TSS = 0;        // Start ADC Timer
                CpuTimer2.RegsAddr->TCR.bit.TSS = 0;        // Start FFT Sync Timer
                EINT; //re-enable interrupts after LCD has been updated
            }
            if(FFTComputeFlag == 1){ // Last resort for if CLA interrupts aren't working
                FFTComputeFlag = 0;
                while (CLA_getTaskRunStatus(CLA1_BASE, CLA_TASK_1) == 1); // wait for CLA to compute the FFT
                FFTDoneFlag = 1;
            }
            if(FFTDoneFlag == 1){
                FFTDoneFlag = 0;
                // Restart the ADC
                for(Uint16 i = 1; i < 2*NSAMPLES; i++){ // i start at 1 to remove DC component
                    FFTMag[i] = (Uint16)abs(IOBuffer2[i]);
                }
                asm(" NOP"); // For Debug breakpoint
                for(Uint16 i = 0; i < 2*NSAMPLES + 2  ; i++) // clear input and output buffers
                {
                    IOBuffer[i] = 0.0;
                    IOBuffer2[i] = 0.0;
                }
                CpuTimer1.RegsAddr->TCR.bit.TSS = 0;        // Start ADC Timer to take next FFT as soon as possible
                //## FFT PROCESSING ##//
                DecimateFFT(FFTMag, 40, 2,Decimate);
                Spectrum(Decimate, LEDSize, r, g, b);
                MyLEDArray(r, g, b);
            }

            break;
        }
    }
}

//*****************************************************************************
// ISR
//*****************************************************************************
interrupt void ADC_ISR(void){

    ADCBuff[IOBufferCnt] = (AdcaResultRegs.ADCRESULT0);
    IOBuffer[IOBufferCnt] = ((float)(AdcaResultRegs.ADCRESULT0)*(3.3/4096.0) - 1.695);
    IOBufferCnt++;

    if(IOBufferCnt == 2*NSAMPLES - 1){
        IOBufferCnt = 0;
        // display the maximum magnitude
        LEDBar(ADCBuff, 2*NSAMPLES, 300);
        CpuTimer1.RegsAddr->TCR.bit.TSS = 1;    // Stop ADC Timer
    }

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear ADC interrupt flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void FFTSync_ISR(void){
    // Compute the FFT
    asm("  IACK  #0x0001");
    asm("  RPT #3 || NOP");
    FFTComputeFlag = 1; // Last resort if CLA interrupts aren't working
}

interrupt void Record_ISR(void){
    DINT; // Disable Interrupts
    DELAY_US(50000);
    if(Task == Idle){
        Task = Record;
        RecordFlag = 1;
        UpdateLCDFlag = 1;
    }

    EINT; // Re-enable Interrupts
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}


interrupt void Tempo_ISR(void){

}

interrupt void Play_ISR(void){
    DINT;
    DELAY_US(20000);
    if(Task == Idle){
        Task = Play;
        PlayFlag = 1;
        UpdateLCDFlag = 1;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
    EINT;
}

interrupt void Prev_ISR(void){
    DINT;
    DELAY_US(20000);
    if(Task == Idle){
    Task = Prev;
    UpdateLCDFlag = 1;
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
    EINT;
}

interrupt void Next_ISR(void){
    DINT;
    DELAY_US(20000);
    if(Task == Idle){
    Task = Next;
    UpdateLCDFlag = 1;
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
    EINT;
}

__interrupt void cla1Isr1 ()
{
    // Signal to CPU1 that the FFT is done being computed
    FFTDoneFlag = 1;

    // Acknowledge the end-of-task interrupt for task 1
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
//  asm(" ESTOP0");
}


__interrupt void cla1Isr2 ()
{
    asm(" ESTOP0");

}
