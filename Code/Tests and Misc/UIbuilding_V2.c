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

enum Uicommand {
    Record      = 1,
    Play        = 2,
    Prev        = 3,
    Next        = 4,
    Idle        = 5
};

enum Uicommand Task = Idle;

Uint16 PlayFlag     = 0;
Uint16 RecordFlag   = 0;
Uint16 BufferFullFlag   = 0;
Uint16 UpdateLCDFlag    = 1;

Uint16 TrackNum         = 0;
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

    while(1){
        switch(Task){
        case Record:
            if(UpdateLCDFlag){
                DINT; // Disable interrupts when LCD is receiving commands
                UpdateLCDFrame(TrackNum, PlayFlag, RecordFlag);
                UpdateLCDFlag = 0; // LCD has been updated and Record/play/track change can happen
                EINT; //re-enable interrupts after LCD has been updated
            }
            DELAY_US(2000000); // delay 2 seconds to roughly simulate a track playing

            RecordFlag = 0;
            UpdateLCDFlag = 1;
            Task = Idle;
            break;
        case Play:
            if(UpdateLCDFlag){
                DINT; // Disable interrupts when LCD is receiving commands
                UpdateLCDFrame(TrackNum, PlayFlag, RecordFlag);
                UpdateLCDFlag = 0; // LCD has been updated and Record/play/track change can happen
                EINT; //re-enable interrupts after LCD has been updated
            }
            DELAY_US(2000000); // delay 2 seconds to roughly simulate a track playing

            PlayFlag = 0;
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
                EINT; //re-enable interrupts after LCD has been updated
            }

            break;
        }


    }

}
// End of main

//*****************************************************************************
// ISR
//*****************************************************************************
interrupt void Play_ISR(void){
    DINT;
    DELAY_US(10000);
    EINT;
    if(Task == Play){
        Task = Idle;
        PlayFlag = 0;
        UpdateLCDFlag = 1;
    }else if(Task == Idle){
        Task = Play;
        PlayFlag = 1;
        UpdateLCDFlag = 1;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void Record_ISR(void){
    DINT;
    DELAY_US(10000);
    EINT;
    if(Task == Record){
        Task = Idle;
        RecordFlag = 0;
        UpdateLCDFlag = 1;
    }else if(Task == Idle){
        Task = Record;
        RecordFlag = 1;
        UpdateLCDFlag = 1;
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void Prev_ISR(void){
    DINT;
    DELAY_US(10000);
    EINT;
    if(Task == Idle){
    Task = Prev;
    UpdateLCDFlag = 1;
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}

interrupt void Next_ISR(void){
    DINT;
    DELAY_US(10000);
    EINT;
    if(Task == Idle){
    Task = Next;
    UpdateLCDFlag = 1;
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}

interrupt void ADC_ISR(void){
    asm(" ESTOP0");
}


