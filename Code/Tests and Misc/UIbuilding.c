//*****************************************************************************
// includes
//*****************************************************************************
#include <F28x_Project.h>
#include "F2837xD_device.h"
#include "device.h"
#include <stdio.h>
#include "OneToOneI2CDriver.h"
#include "ukemasterfunc.h"
#include "ukemasterdrivers.h"

#define TrackLength 1000    // total number of notes to send to the FPGA
#define NumTracks 3         // The total number of tracks

//// Push Button ISR Definitions
//interrupt void Play_ISR(void);
//interrupt void Record_ISR(void);
//interrupt void Prev_ISR(void);
//interrupt void Next_ISR(void);

Uint16 RecordFlag       = 0;
Uint16 BufferFullFlag   = 0; //handles case of recording ending before Record button is put back in a neutral state
Uint16 PlayFlag         = 0;
Uint16 PrevFlag         = 0;
Uint16 NextFlag         = 0;
Uint16 UpdateLCDFlag    = 0;

Uint16 TrackNum = 0;
Uint16 Track0[TrackLength];
Uint16 Track1[TrackLength];
Uint16 Track2[TrackLength];
int main(void)
{

    InitSysCtrl();
//    CLA_DSP_initSystemClocks();

    EALLOW;             // allows ADC Registers to be modified

    GpioCtrlRegs.GPADIR.bit.GPIO19 = 1; // Set pin as output (pin 3 on launchpad)
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

    GPIOInit();

    EnableInterrupts(); // Enable PIE and CPU interrupts. Includes EINT

    UpdateLCDFlag = 1; // Update LCD to show initial Frame
    while(1){
        if(UpdateLCDFlag){
            DINT; // Disable interrupts when LCD is receiving commands
            UpdateLCDFrame(TrackNum, PlayFlag, RecordFlag);
            UpdateLCDFlag = 0; // LCD has been updated and Record/play/track change can happen
            EINT; //re-enable interrupts after LCD has been updated
        }

        // Record Press Routine
        while(RecordFlag && !BufferFullFlag && !UpdateLCDFlag){ // can only record if recording requested, nothing is playing, and the LCD has been updated
            DELAY_US(2000000); // delay 2 seconds to roughly simulate a track recording
            BufferFullFlag = 1; // PC has reached end of recording routine and the buffer is now full
            UpdateLCDFlag = 1;
        }
        // Play Press Routine
        while(PlayFlag && !UpdateLCDFlag){ // can only play if playing requested, nothing is recording, and the LCD has been updated
            DELAY_US(2000000); // delay 2 seconds to roughly simulate a track playing

            //if(notes_played == TrackLength)...{ or something like this
            PlayFlag = 0; // play flag goes to zero after song is played
            UpdateLCDFlag = 1; // Update LCD to show that the song has finished playing
            //}

        }
        // Next Press Routine
        if(NextFlag && !UpdateLCDFlag){
            if(TrackNum == (NumTracks - 1)){    // go to the first track if increment past the # of tracks
                TrackNum = 0;
            }else{
                TrackNum++;
            }
            NextFlag = 0; // clear NextFlag
            UpdateLCDFlag = 1; // update the LCD to show the new track
        }
        // Prev Press Routine
        if(PrevFlag && !UpdateLCDFlag){
            if(TrackNum == 0){    // go to the last track if decrement past 0
                TrackNum = (NumTracks - 1);
            }else{
                TrackNum--;
            }
            PrevFlag = 0; // clear NextFlag
            UpdateLCDFlag = 1; // update the LCD to show the new track
        }

    }

}
// End of main

//*****************************************************************************
// ISR
//*****************************************************************************
interrupt void Play_ISR(void){
    if(!RecordFlag && !PrevFlag && !NextFlag && !UpdateLCDFlag){
        PlayFlag = !PlayFlag;
        UpdateLCDFlag = 1; // signal that the LCD can update
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void Record_ISR(void){
    if(!PlayFlag && !PrevFlag && !NextFlag && !UpdateLCDFlag){
        RecordFlag = !RecordFlag;
        UpdateLCDFlag = 1; // signal that the LCD can update
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void Prev_ISR(void){
    if(!PlayFlag && !RecordFlag && !NextFlag && !UpdateLCDFlag){
        PrevFlag = 1;
        UpdateLCDFlag = 1; // signal that the LCD can update
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}

interrupt void Next_ISR(void){
    if(!PlayFlag && !RecordFlag && !PrevFlag && !UpdateLCDFlag){
        NextFlag = 1;
        UpdateLCDFlag = 1; // signal that the LCD can update
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}

interrupt void ADC_ISR(void){
    asm(" ESTOP0");
}


