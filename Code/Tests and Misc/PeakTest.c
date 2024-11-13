//*****************************************************************************
// includes
//*****************************************************************************
#include <F28x_Project.h>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include "f2837x_fft_examples_setup.h"
#include "F2837xD_device.h"
#include "rfft_1024_shared.h"
#include "OneToOneI2CDriver.h"
#include "ukemasterfunc.h"
#include "ukemasterdrivers.h"
#include "device.h"


                  //0 1 2 3  4  5 6 7 8 9 10 11 12 13 14 15
Uint16 Peaks[16] = {0,0,1,18,17,0,0,0,7,0,8, 9, 4, 2, 0, 6};
Uint16 Decimation[8];

int main(void){


    InitSysCtrl();
//    CLA_DSP_initSystemClocks();

    Uint16 PeakMags[NumPeaks];
    Uint16 PeakBins[NumPeaks];

    FindPeaks(Peaks, 0, 16, 2, PeakMags, PeakBins);
    DecimateFFT(Peaks, 0, 2,Decimation);

    for(Uint16 i = 0; i < NumPeaks; i++){
        PeakMags[i] = 0;
        PeakBins[i] = 0;
        Decimation[i] = 0;
    }

    asm(" NOP");
}
