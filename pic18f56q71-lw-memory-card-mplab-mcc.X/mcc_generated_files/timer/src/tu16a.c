/**
 * TU16A Generated Driver File
 *
 * @file tu16a.c
 *  
 * @ingroup tu16a
 *
 * @brief This file contains the API definitions for the TU16A module.
 *
 * @version TU16A Driver Version 2.1.0
 */

 /*
© [2024] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

#include <xc.h>
#include "../tu16a.h"

const struct TMR_INTERFACE TU16A = {
    .Initialize = TU16A_Initialize,
    .Start = TU16A_Start,
    .Stop = TU16A_Stop,
    .PeriodCountSet = TU16A_Write,
    .TimeoutCallbackRegister = NULL,
    .Tasks = NULL
};

void TU16A_Initialize(void)
{
    //Stop Timer
    TU16ACON0bits.ON = 0;

    //CIF disabled; ZIF disabled; PRIF disabled; CAPT disabled; LIMIT disabled; CLR disabled; OSEN enabled;  
    TU16ACON1 = 0x40;
    //STOP At PR Match; RESET At Start and PR match; START No hardware Start (Always On); CSYNC enabled; EPOL non inverted;  
    TU16AHLT = 0x4B;
    //CLK LFINTOSC;   
    TU16ACLK = 0x4;
    //ERS TU16APRL_Write;  
    TU16AERS = 0x1F;
    //PS 31;  
    TU16APS = 0x1F;
    //PRH 0; 
    TU16APRH = 0x0;
    //PRL 243; 
    TU16APRL = 0xF3;
    //TU16BPR Low Byte
    TU16BPRL = 0x0;
    //TU16BPRH High Byte
    TU16BPRH = 0x0;
    //TMRH 0; 
    TU16ATMRH = 0x0;
    //TMRL 0; 
    TU16ATMRL = 0x0;
    //TU16BTMR Low Byte
    TU16BTMRL = 0x0;
    //TU16BTMRH High Byte
    TU16BTMRH = 0x0;

    //CH16AB enabled;  
    TUCHAIN = 0x1;
    //disable TUI interrupt
    PIE5bits.TU16AIE = 0;

    //CIE disabled; ZIE disabled; PRIE disabled; RDSEL read; OPOL low; OM pulse mode; CPOL rising edge; ON disabled;  
    TU16ACON0 = 0x40;
}

void TU16A_Start(void)
{
    TU16ACON0bits.ON = 1;
}

void TU16A_Stop(void)
{
    TU16ACON0bits.ON = 0;
}

uint32_t TU16A_CaptureValueRead(void)
{
    TU16ACON0bits.RDSEL = 0;
    return (uint32_t)(((uint32_t)TU16BCRH << 24) | ((uint32_t)TU16BCRL << 16) | ((uint32_t)TU16ACRH<< 8) | TU16ACRL);
}

uint32_t TU16A_OnCommandCapture(void)
{
    TU16ACON1bits.CAPT = 1;
    while(TU16ACON1bits.CAPT == 1);
    return TU16A_CaptureValueRead();
}

uint32_t TU16A_Read(void)
{
    bool onVal = TU16ACON0bits.ON;
    TU16ACON0bits.ON = 0;
    TU16ACON0bits.RDSEL = 1;
    uint32_t tmrVal = (uint32_t)(((uint32_t)TU16BTMRH << 24) | ((uint32_t)TU16BTMRL << 16) | ((uint32_t)TU16ATMRH<< 8) | TU16ATMRL);
    TU16ACON0bits.ON = onVal;
    return tmrVal;
}

void TU16A_Write(size_t timerVal)
{
    uint32_t timerValGet = (uint32_t) timerVal;
    bool onVal = TU16ACON0bits.ON;
    TU16ACON0bits.ON = 0;
    TU16BTMRH = (uint8_t) ((timerValGet >> 24) & 0xFF);
    TU16BTMRL = (uint8_t) ((timerValGet >> 16) & 0xFF);
    TU16ATMRH = (uint8_t) (timerValGet >> 8);
    TU16ATMRL = (uint8_t) (timerValGet & 0xFF);
    TU16ACON0bits.ON = onVal;
}

void TU16A_CounterClear(void)
{
    TU16ACON1bits.CLR = 1;
    while(TU16ACON1bits.CLR == 1);
}

void TU16A_PeriodValueSet(uint32_t prVal)
{
    TU16BPRH = (uint8_t)((prVal >> 24) & 0xFF);
    TU16BPRL = (uint8_t)((prVal >> 16) & 0xFF);
    TU16APRH = (uint8_t)((prVal >> 8) & 0xFF);
    TU16APRL = (uint8_t)(prVal & 0xFF);
}

void TU16A_PRMatchInterruptEnable(void)
{
    TU16ACON0bits.PRIE = 1;
}

void TU16A_PRMatchInterruptDisable(void)
{
    TU16ACON0bits.PRIE = 0;
}

void TU16A_ZeroInterruptEnable(void)
{
    TU16ACON0bits.ZIE = 1;
}

void TU16A_ZeroInterruptDisable(void)
{
    TU16ACON0bits.ZIE = 0;
}

void TU16A_CaptureInterruptEnable(void)
{
    TU16ACON0bits.CIE = 1;
}

void TU16A_CaptureInterruptDisable(void)
{
    TU16ACON0bits.CIE = 0;
}

bool TU16A_HasPRMatchOccured(void)
{
    return TU16ACON1bits.PRIF;
}

bool TU16A_HasResetOccured(void)
{
    return TU16ACON1bits.ZIF;
}

bool TU16A_HasCaptureOccured(void)
{
    return TU16ACON1bits.CIF;
}

bool TU16A_IsTimerRunning(void)
{
    return TU16ACON1bits.RUN;
}

void TU16A_InterruptEnable(void)
{
    PIE5bits.TU16AIE = 1;
}

void TU16A_InterruptDisable(void)
{
    PIE5bits.TU16AIE = 0;
}

bool TU16A_IsInterruptEnabled(void)
{
    return PIE5bits.TU16AIE;
}

void TU16A_InterruptFlagsClear(void)
{
    TU16ACON1bits.PRIF = 0;
    TU16ACON1bits.ZIF = 0;
    TU16ACON1bits.CIF = 0;
}

