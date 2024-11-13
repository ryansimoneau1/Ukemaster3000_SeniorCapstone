/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef BOARD_H
#define BOARD_H

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//
// Included Files
//

#include "driverlib.h"
#include "device.h"

//*****************************************************************************
//
// PinMux Configurations
//
//*****************************************************************************

//
// OUTPUTXBAR1 -> myOUTPUTXBAR1 Pinmux
//
//
// OUTPUTXBAR1 - GPIO Settings
//
#define GPIO_PIN_OUTPUTXBAR1 24
#define myOUTPUTXBAR1_OUTPUTXBAR_GPIO 24
#define myOUTPUTXBAR1_OUTPUTXBAR_PIN_CONFIG GPIO_24_OUTPUTXBAR1

//*****************************************************************************
//
// CLB Configurations
//
//*****************************************************************************
#define myCLBTILE1_BASE CLB1_BASE
void myCLBTILE1_init();
//
// Tile Configurations for all CLBs are in this file
//
#include "clb_config.h"

//*****************************************************************************
//
// CLBXBAR Configurations
//
//*****************************************************************************
void myCLBXBAR0_init();
#define myCLBXBAR0 XBAR_AUXSIG0

#define myCLBXBAR0_ENABLED_MUXES (XBAR_MUX01)
void myCLBXBAR1_init();
#define myCLBXBAR1 XBAR_AUXSIG1

#define myCLBXBAR1_ENABLED_MUXES (XBAR_MUX03)

//*****************************************************************************
//
// INPUTXBAR Configurations
//
//*****************************************************************************
#define myINPUTXBARINPUT1_SOURCE 65
#define myINPUTXBARINPUT1_INPUT XBAR_INPUT1
void myINPUTXBARINPUT1_init();
#define myINPUTXBARINPUT2_SOURCE 63
#define myINPUTXBARINPUT2_INPUT XBAR_INPUT2
void myINPUTXBARINPUT2_init();

//*****************************************************************************
//
// OUTPUTXBAR Configurations
//
//*****************************************************************************
void myOUTPUTXBAR1_init();
#define myOUTPUTXBAR1 XBAR_OUTPUT1
#define myOUTPUTXBAR1_ENABLED_MUXES (XBAR_MUX01)

//*****************************************************************************
//
// Board Configurations
//
//*****************************************************************************
void    Board_init();
void    CLB_init();
void    CLBXBAR_init();
void    INPUTXBAR_init();
void    OUTPUTXBAR_init();
void    PinMux_init();

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif  // end of BOARD_H definition
