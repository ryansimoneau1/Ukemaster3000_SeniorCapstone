#include "ukemasterfunc.h"
#include "ukemasterdrivers.h"


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
Uint16 SortNotes(Uint16 *bins)
{
    //string flags
    Uint16 string1_flag = 0;
    Uint16 string2_flag = 0;
    Uint16 string3_flag = 0;
    Uint16 string4_flag = 0;

    //data from each string, defaults are 111 = "No note"
    Uint16 string1_data = 0b1110000000000000;
    Uint16 string2_data = 0b0001110000000000;
    Uint16 string3_data = 0b0000001110000000;
    Uint16 string4_data = 0b0000000001110000;

    //output data, combination of each string
    Uint16 data = string1_data | string2_data | string3_data | string4_data;

    for(Uint16 i = 0; i < NumPeaks; i++) {
        //strings ordered lowest, highest, second highest, second lowest frequency
        //check if note is in lowest string (string 2)
        if ((bins[i] >= B3) && (bins[i] < F4) && (string2_flag == 0)) { //string 2
            //mark flag
            string2_flag = 1;
            //divide into specific notes
            if ((bins[i] >= B3) && (bins[i] < C4)) { //open note
                string2_data = 0b0000010000000000;
            } else if ((bins[i] >= C4) && (bins[i] < Db4)) {//fret 1
                string2_data = 0b0000100000000000;
            } else if ((bins[i] >= Db4) && (bins[i] < D4)) {//fret 2
                string2_data = 0b0000110000000000;
            } else if ((bins[i] >= D4) && (bins[i] < Eb4)) {//fret 3
                string2_data = 0b0001000000000000;
            } else if ((bins[i] >= Eb4) && (bins[i] < E4)) {//fret 4
                string2_data = 0b0001010000000000;
            } else { //up to F4, fret 5
                string2_data = 0b0001100000000000;
            }
        }
        else if ((bins[i] >= Eb4) && (bins[i] < A4) && (string3_flag == 0)) { // string 3
            //mark flag
            string3_flag = 1;
            //divide into specific notes
            if ((bins[i] >= Eb4) && (bins[i] < E4)) {//open note
                string3_data = 0b0000000010000000;
            } else if ((bins[i] >= E4) && (bins[i] < F4)) {//fret 1
                string3_data = 0b0000000100000000;
            } else if ((bins[i] >= F4) && (bins[i] < Gb4)) {//fret 2
                string3_data = 0b0000000110000000;
            } else if ((bins[i] >= Gb4) && (bins[i] < G4)) {//fret 3
                string3_data = 0b0000001000000000;
            } else if ((bins[i] >= G4) && (bins[i] < Ab4)) {//fret 4
                string3_data = 0b0000001010000000;
            } else { //up to A4, fret 5
                string3_data = 0b0000001100000000;
            }
        }
        else if ((bins[i] >= Gb4) && (bins[i] < C5) && (string1_flag == 0)) { // string 1
            //mark flag
            string1_flag = 1;
            //divide into specific notes
            if ((bins[i] >= Gb4) && (bins[i] < G4)) {//open note
                string1_data = 0b0010000000000000;
            } else if ((bins[i] >= G4) && (bins[i] < Ab4)) {//fret 1
                string1_data = 0b0100000000000000;
            } else if ((bins[i] >= Ab4) && (bins[i] < A4)) {//fret 2
                string1_data = 0b0110000000000000;
            } else if ((bins[i] >= A4) && (bins[i] < Bb4)) {//fret 3
                string1_data = 0b1000000000000000;
            } else if ((bins[i] >= Bb4) && (bins[i] < B4)) {//fret 4
                string1_data = 0b1010000000000000;
            } else { //up to E5, fret 5
                string1_data = 0b1100000000000000;
            }
        }
        else if ((bins[i] >= Ab4) && (bins[i] <= D5) && (string4_flag == 0)) { // string 4
            //mark flag
            string4_flag = 1;
            //divide into specific notes
            if ((bins[i] >= Ab4) && (bins[i] < A4)) {//open note
                string4_data = 0b0000000000010000;
            } else if ((bins[i] >= A4) && (bins[i] < Bb4)) {//fret 1
                string4_data = 0b0000000000100000;
            } else if ((bins[i] >= Bb4) && (bins[i] < B4)) {//fret 2
                string4_data = 0b0000000000110000;
            } else if ((bins[i] >= B4) && (bins[i] < C5)) {//fret 3
                string4_data = 0b0000000001000000;
            } else if ((bins[i] >= C5) && (bins[i] < Db5)) {//fret 4
                string4_data = 0b0000000001010000;
            } else { //up to D5, fret 5
                string4_data = 0b0000000001100000;
            }
        }
    }

    //once all strings and frets are found, combine them
    data = string1_data | string2_data | string3_data | string4_data;

    return data;
}

/*
 * Summary: Sends SPI data to the FPGA given an input array of data and its size. Sends each string of bits twice and checks
 *          for FPGA to acknowledge no corruption has occurred. If corruption present, keeps sending data until not corrupted
 *
 * sdata: The input array, each element in the array is a string of bits to be sent to FPGA
 * sdelay: The delay between each element in sdata. In other words, time before each new
 *          note/chord is played on the uke
 * size: size of both arrays (should be the same)
 */
void SendNotes(Uint16 *sdata, Uint16 size)
{
    // chip select goes high, default state
    GPIO_WritePin(19, 1);

    // rst pin goes low
    GPIO_WritePin(67, 0);

    //initialize error flag
    Uint16 comp_flag = 0;

    int i = 1;
    for (i = 3; i <= size; i++)
    {
        //first send
        //chip select low
        GPIO_WritePin(19, 0);
        //transmit data
        SpiTransmit_FPGA(sdata[i-1]);
//        SpiTransmit_FPGA(0b1111111111110000);
        DELAY_US(2);
        //chip select high
        GPIO_WritePin(19, 1);
        //second send
        //CS low
        GPIO_WritePin(19, 0);
        //send data
        SpiTransmit_FPGA(sdata[i-1]);
//        SpiTransmit_FPGA(0b1111111111110000);
        DELAY_US(2);
        //check corruption flag, decrement counter if corrupted
        comp_flag = GPIO_ReadPin(32);
        GPIO_WritePin(19, 1);
        if (comp_flag == 1)
        {
            i--;
            comp_flag = 0;
        } else {
            //delaying for the next note/chord to be played
            DELAY_US(100000);
        }
    }

    //pull rst high again
    GPIO_WritePin(67, 1);
}

/*
 * Summary: Finds a number of peaks in the FFT as defined by the user. Additionally allows user to configure the number of
 *          bins away from a found peak to start searching for the next peak. Sorts all peaks in descending order by magnitude
 *
 * FFTmagnitude: The Raw FFT magnitude data
 * StartBin: Start of interval to look for peaks in
 * StopBin: End of interval to look for peaks in
 * NumPeaks: Number of peaks to find
 * PeakWidth: Number of bins away from a found peak that another peak can be found
 */
void FindPeaks(Uint16 *FFTmagnitude, Uint16 StartBin, Uint16 StopBin, Uint16 PeakWidth, Uint16 *PeakMags, Uint16 *PeakBins){
    //temporary but sort by freq and not mag
    Uint16 tempMags[NumPeaks];
    Uint16 tempBins[NumPeaks];
    Uint16 temp = 0;

    //setting all elements to 0
    for (int i = 0; i < NumPeaks; i++) {
        tempMags[i] = 0;
        tempBins[i] = 0;
    }
    for (Uint16 i = 0; i < NumPeaks; i++) {
        for (Uint16 j = StartBin; j < StopBin; j++) {
//            if ((FFTmagnitude[j] > tempMags[i]) && (FFTmagnitude[j] > MinPeakValue)) {
            if ((FFTmagnitude[j] > tempMags[i]) && (((j <= 72) && (FFTmagnitude[j] > (MinPeakValue*0.65))) || ((j > 72) && (FFTmagnitude[j] > MinPeakValue))) ) {
            // check if new peak is larger than ones that are close to it
                Uint16 k;
                for (k = 0; k < i; k++) { // checks all 10 peaks. Orders the buffer in descending order
                    if (abs(j - tempBins[k]) < PeakWidth) {
                    break;
                    }
                }
                if (k == i) {
                tempMags[i] = FFTmagnitude[j];
                tempBins[i] = j;
                }
            }
        }
    }
    // copying temporary arrays to output ones
    for (Uint16 i = 0; i < NumPeaks; i++) {
        PeakMags[i] = tempMags[i];
        PeakBins[i] = tempBins[i];
    }

    // sorting bins in ascending order
    for (Uint16 i = 0; i < 4; i++) {
        for (Uint16 j = i+1; j < 4; j++) {
            if (PeakBins[i] > PeakBins[j]) {
                temp = PeakBins[i];
                PeakBins[i] = PeakBins[j];
                PeakBins[j] = temp;
            }
        }
    }

    // reflecting that sorting in PeakMags
    for (Uint16 i = 0; i < 4; i++) {
        for (Uint16 j = 0; j < 4; j++) {
            if (PeakBins[i] == tempBins[j]) {
                PeakMags[i] = tempMags[j];
                break;
            }
        }
    }
}

/*
 * Summary: Takes FFT values and decimates them to the number of LEDs available in strip. Allows for range of frequencies to be
 *          based on target # of Bins per LED and LED strip Size.
 *
 * FFTmag: FFT Magnitude data computed by CLA
 * StartBin: Starting frequency for output
 * BinsPerLED: Target # of bins per LED
 * LEDdata: Pass by reference output values
 */
void DecimateFFT(Uint16 *FFTmag, Uint16 StartBin, Uint16 BinsPerLED, Uint16 *LEDdata){

    for(Uint16 i = 0; i < LEDSize; i++){
        Uint16 DecLocation = BinsPerLED*i + StartBin; // Where in the FFTmag buffer we are taking values to decimate
        for(Uint16 j = DecLocation; j < (DecLocation + BinsPerLED); j++){
            LEDdata[i] += FFTmag[j];
        }
        LEDdata[i] = (Uint16)((float)LEDdata[i]/(float)BinsPerLED); // Average the values and store in LEDdata
    }
}

// FFTDecimated to RGB values
void Spectrum(Uint16 *FFTVals, Uint16 Size, uint8_t *r, uint8_t *g, uint8_t *b){
    float Slope = ColorSlope;
    for(uint8_t i = 0; i < Size; i++){
        float MagVal = (float)FFTVals[i];
        if(MagVal <= ZeroBin){ // 0
            r[i] = 0;
            g[i] = 0;
            b[i] = 0;
        } else if(MagVal <= (ZeroBin + 1*MagBin)){ // 1
            r[i] = (uint8_t)(ColorBrightness - Slope*MagVal);
            g[i] = 0;
            b[i] = ColorBrightness;
        } else if(MagVal <= (ZeroBin + 2*MagBin)){ // 2
            r[i] = 0;
            g[i] = (uint8_t)(Slope*MagVal);
            b[i] = ColorBrightness;
        } else if(MagVal <= (ZeroBin + 3*MagBin)){ // 3
            r[i] = 0;
            g[i] = ColorBrightness;
            b[i] = (uint8_t)(ColorBrightness - Slope*MagVal);
        } else if(MagVal <= (ZeroBin + 4*MagBin)){ // 4
            r[i] = (uint8_t)(Slope*MagVal);
            g[i] = ColorBrightness;
            b[i] = 0;
        } else if(MagVal <= (ZeroBin + 5*MagBin)){ // 5
            r[i] = ColorBrightness;
            g[i] = (uint8_t)(ColorBrightness - Slope*MagVal);
            b[i] = 0;
        }

    }
}

void MyLEDArray(Uint16 *r, Uint16 *g, Uint16 *b){ // send data for the entire LED strip
    volatile Uint16 dummyLoad = 0;
    for(Uint16 i = 0; i < LEDSize; i++){
        SpibRegs.SPIDAT = g[i] << 8; //send data to SPI register
        while (SpibRegs.SPISTS.bit.INT_FLAG == 0); //wait until the data has been sent
        dummyLoad = SpibRegs.SPIRXBUF; //reset flag

        SpibRegs.SPIDAT = r[i] << 8; //send data to SPI register
        while (SpibRegs.SPISTS.bit.INT_FLAG == 0); //wait until the data has been sent
        dummyLoad = SpibRegs.SPIRXBUF; //reset flag

        SpibRegs.SPIDAT = b[i] << 8; //send data to SPI register
        while (SpibRegs.SPISTS.bit.INT_FLAG == 0); //wait until the data has been sent
        dummyLoad = SpibRegs.SPIRXBUF; //reset flag
    }
}


void shift_array(Uint16 *arr){
    int last_value = arr[LEDSize - 1];  // get the last value
    for (Uint16 i = LEDSize - 1; i > 0; i--) {
        arr[i] = arr[i - 1];  // shift elements to the right
    }
    arr[0] = last_value;  // replace the first value with the last
}

/*
 * Summary: Finds Max value in microphone signal and lights up LEDs on LED bar to signify how close the input is to the ADC rails.
 *          Size corresponds to the number of samples in ADCSamples that are used to find the max value of the input signal. Larger
 *          size will improve the accuracy of the function, but the process will take longer.
 *
 * ADCSamples: Microphone ADC Buffer holding audio values
 * Size: Amount of buffer used to find the max value
 * LowLimit: The minimum value for the LED bar to show
 */
void LEDBar(Uint16 *ADCSamples, Uint16 Size, Uint16 LowLimit){

    Uint16 LowVal = LowLimit + 2048; // account for 1.65 V DC offset
    Uint16 MaxValue = 0;

    for(Uint16 i = 0; i < Size; i++){
        if(ADCSamples[i] > MaxValue){
            MaxValue = ADCSamples[i];
        }
    }
    if(MaxValue < LowVal){
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
    }else if(MaxValue >= LowVal && MaxValue < LEDBoundary1){
        OnLED1;
        OffLED2;
        OffLED3;
        OffLED4;
        OffLED5;
        OffLED6;
        OffLED7;
        OffLED8;
        OffLED9;
        OffLED10;
    }else if(MaxValue >= LEDBoundary1 && MaxValue < LEDBoundary2){
        OnLED1;
        OnLED2;
        OffLED3;
        OffLED4;
        OffLED5;
        OffLED6;
        OffLED7;
        OffLED8;
        OffLED9;
        OffLED10;
    }else if(MaxValue >= LEDBoundary2 && MaxValue < LEDBoundary3){
        OnLED1;
        OnLED2;
        OnLED3;
        OffLED4;
        OffLED5;
        OffLED6;
        OffLED7;
        OffLED8;
        OffLED9;
        OffLED10;
    }else if(MaxValue >= LEDBoundary3 && MaxValue < LEDBoundary4){
        OnLED1;
        OnLED2;
        OnLED3;
        OnLED4;
        OffLED5;
        OffLED6;
        OffLED7;
        OffLED8;
        OffLED9;
        OffLED10;
    }else if(MaxValue >= LEDBoundary4 && MaxValue < LEDBoundary5){
        OnLED1;
        OnLED2;
        OnLED3;
        OnLED4;
        OnLED5;
        OffLED6;
        OffLED7;
        OffLED8;
        OffLED9;
        OffLED10;
    }else if(MaxValue >= LEDBoundary5 && MaxValue < LEDBoundary6){
        OnLED1;
        OnLED2;
        OnLED3;
        OnLED4;
        OnLED5;
        OnLED6;
        OffLED7;
        OffLED8;
        OffLED9;
        OffLED10;
    }else if(MaxValue > LEDBoundary6 && MaxValue < LEDBoundary7){
        OnLED1;
        OnLED2;
        OnLED3;
        OnLED4;
        OnLED5;
        OnLED6;
        OnLED7;
        OffLED8;
        OffLED9;
        OffLED10;
    }else if(MaxValue > LEDBoundary7 && MaxValue < LEDBoundary8){
        OnLED1;
        OnLED2;
        OnLED3;
        OnLED4;
        OnLED5;
        OnLED6;
        OnLED7;
        OnLED8;
        OffLED9;
        OffLED10;
    }else if(MaxValue > LEDBoundary8 && MaxValue < LEDBoundary9){
        OnLED1;
        OnLED2;
        OnLED3;
        OnLED4;
        OnLED5;
        OnLED6;
        OnLED7;
        OnLED8;
        OnLED9;
        OffLED10;
    }else if(MaxValue > LEDBoundary9 && MaxValue < LEDBoundary10){
        OnLED1;
        OnLED2;
        OnLED3;
        OnLED4;
        OnLED5;
        OnLED6;
        OnLED7;
        OnLED8;
        OnLED9;
        OnLED10;
    }
}


/*
 * Summary: Function decides what is displayed on the LCD based on what track is selected and wether the user wants to play or
 *          record a song
 *
 * TrackNumber: The track being played or recorded
 * PlayFlag: means that the song is going to be played
 * RecordFlag: means the song is being recorded
 * DoneFlag: means that the song has been completed and the track should go back to idle state
 */
void UpdateLCDFrame(Uint16 TrackNumber, Uint16 PlayFlag, Uint16 RecordFlag){

    char Track0[16]     = {'T','r','a','c','k',':',' ','0',' ',' ',' ',' ',' ',' ',' ',' '};
    char Track1[16]     = {'T','r','a','c','k',':',' ','1',' ',' ',' ',' ',' ',' ',' ',' '};
    char Track2[16]     = {'T','r','a','c','k',':',' ','2',' ',' ',' ',' ',' ',' ',' ',' '};
    char Track3[16]     = {'D','E','M','O',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    char Track4[16]     = {'L','E','D',' ','D','E','M','O',' ',' ',' ',' ',' ',' ',' ',' '};
    char PlayLCD[16]    = {'P','l','a','y','i','n','g','.','.','.',' ',' ',' ',' ',' ',' '};
    char RecordLCD[16]  = {'R','e','c','o','r','d','i','n','g','.','.','.',' ',' ',' ',' '};
    char BlankLCD[16]   = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

    //Reset LCD when changes are made
    LCDCTRL(0x02);

    switch(TrackNumber){

    case 0:
        LCDSTRING(Track0,16);
        LCDCTRL(0xC0); // go to the next line
        if(!PlayFlag && !RecordFlag){
            // display default track 0 screen
            LCDSTRING(BlankLCD,16);

        }else if(PlayFlag && !RecordFlag){
            // display track 0 Playing screen
            LCDSTRING(PlayLCD,16);

        }else if(!PlayFlag && RecordFlag){
            // display track 0 Recording screen
            LCDSTRING(RecordLCD,16);

        }
    case 1:
        LCDSTRING(Track1,16);
        LCDCTRL(0xC0); // go to the next line
        if(!PlayFlag && !RecordFlag){
            // display default track 1 screen
            LCDSTRING(BlankLCD,16);

        }else if(PlayFlag && !RecordFlag){
            // display track 1 Playing screen
            LCDSTRING(PlayLCD,16);

        }else if(!PlayFlag && RecordFlag){
            // display track 1 Recording screen
            LCDSTRING(RecordLCD,16);

        }
    case 2:
        LCDSTRING(Track2,16);
        LCDCTRL(0xC0); // go to the next line
        if(!PlayFlag && !RecordFlag){
            // display default track 2 screen
            LCDSTRING(BlankLCD,16);

        }else if(PlayFlag && !RecordFlag){
            // display track 2 Playing screen
            LCDSTRING(PlayLCD,16);

        }else if(!PlayFlag && RecordFlag){
            // display track 2 Recording screen
            LCDSTRING(RecordLCD,16);
        }
    case 3:
        LCDSTRING(Track3,16);
        LCDCTRL(0xC0); // go to the next line
        if(!PlayFlag && !RecordFlag){
            // display default track 2 screen
            LCDSTRING(BlankLCD,16);

        }else if(PlayFlag && !RecordFlag){
            // display track 2 Playing screen
            LCDSTRING(PlayLCD,16);

        }else if(!PlayFlag && RecordFlag){
            // display track 2 Recording screen
            LCDSTRING(RecordLCD,16);
        }
    case 4:
        LCDSTRING(Track4,16);
        LCDCTRL(0xC0); // go to the next line
        if(!PlayFlag && !RecordFlag){
            // display default track 2 screen
            LCDSTRING(BlankLCD,16);

        }else if(PlayFlag && !RecordFlag){
            // display track 2 Playing screen
            LCDSTRING(PlayLCD,16);

        }else if(!PlayFlag && RecordFlag){
            // display track 2 Recording screen
            LCDSTRING(RecordLCD,16);
        }
    }
}

void LCDCTRL(Uint16 command){

   Uint16 x[4];

   x[0]  = (command & 0xF0) | 0xC;
   x[1]  = (command & 0xF0) | 0x8;

   x[2]  = (command << 4) | 0xC;
   x[2]  = (x[2] & 0x00FF);
   x[3]  = (command << 4) | 0x8;
   x[3]  = (x[3] & 0x00FF);

   I2C_O2O_SendBytes(&x[0], 4);

}


void LCDDATA(Uint16 data){

    Uint16 x[4];

    x[0]  = (data & 0xF0) | 0xD;
    x[1]  = (data & 0xF0) | 0x9;

    x[2]  = (data << 4) | 0xD;
    x[2]  = (x[2] & 0x00FF);
    x[3]  = (data << 4) | 0x9;
    x[3]  = (x[3] & 0x00FF);

    I2C_O2O_SendBytes(&x[0], 4);

}


// function with hardcodded values meant to initialize the LCD
 void LCDINIT(void){


    #define LCDinits    5

    Uint16  LCDI[LCDinits]  = {0x33, 0x32, 0x28, 0x0F, 0x01};

    Uint16 temp[20];
    // 0x33
    temp[0]  = (LCDI[0] & 0xF0) | 0xC;
    temp[1]  = (LCDI[0] & 0xF0) | 0x8;

    temp[2]  = (LCDI[0] << 4) | 0xC;
    temp[2]  = (temp[2] & 0x00FF);
    temp[3]  = (LCDI[0] << 4) | 0x8;
    temp[3]  = (temp[3] & 0x00FF);


    // 0x32
    temp[4]  = (LCDI[1] & 0xF0) | 0xC;
    temp[5]  = (LCDI[1] & 0xF0) | 0x8;

    temp[6]  = (LCDI[1] << 4) | 0xC;
    temp[6]  = (temp[6] & 0x00FF);
    temp[7]  = (LCDI[1] << 4) | 0x8;
    temp[7]  = (temp[7] & 0x00FF);

    // 0x28
    temp[8]  = (LCDI[2] & 0xF0) | 0xC;
    temp[9]  = (LCDI[2] & 0xF0) | 0x8;

    temp[10]  = (LCDI[2] << 4) | 0xC;
    temp[10]  = (temp[10] & 0x00FF);
    temp[11]  = (LCDI[2] << 4) | 0x8;
    temp[11]  = (temp[11] & 0x00FF);

    // 0x0F
    temp[12] = (LCDI[3] & 0xF0) | 0xC;
    temp[13] = (LCDI[3] & 0xF0) | 0x8;

    temp[14] = (LCDI[3] << 4) | 0xC;
    temp[14]  = (temp[14] & 0x00FF);
    temp[15]  = (LCDI[3] << 4) | 0x8;
    temp[15]  = (temp[15] & 0x00FF);

    // 0x01
    temp[16] = (LCDI[4] & 0xF0) | 0xC;
    temp[17] = (LCDI[4] & 0xF0) | 0x8;

    temp[18] = (LCDI[4] << 4) | 0xC;
    temp[18]  = (temp[18] & 0x00FF);
    temp[19]  = (LCDI[4] << 4) | 0x8;
    temp[19]  = (temp[19] & 0x00FF);

    I2C_O2O_SendBytes(&temp[0], 20);

}

 void LCDSTRING(char * const string, Uint16 length){

     for (Uint16 i = 0; i < length; i++)
         {
             LCDDATA(string[i]);
         }


 }


