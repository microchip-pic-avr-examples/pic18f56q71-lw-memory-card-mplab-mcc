/**
 * CRC Generated Driver File.
 * 
 * @file crc.c
 * 
 * @ingroup  crc
 * 
 * @brief This file contains the API implementation for the CRC driver.
 *
 * @version CRC Driver Version 1.0.1
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

/**
  Section: Included Files
*/

#include <xc.h>
#include "../crc.h"



void CRC_Initialize(void)
{
    CRCCON0 = 0x00;
    //CRC Configurations
    //CRCPLEN 15; 
    CRCCON1 = 0xF;
    //CRCDLEN 7; 
    CRCCON2 = 0x7;

    // Read/Write access to CRCXOR
    CRCCON0bits.SETUP = 0b10;
    //CRCXORT 0; 
    CRCXORT = 0x0;
    //CRCXORU 0; 
    CRCXORU = 0x0;
    //CRCXORH 16; 
    CRCXORH = 0x10;
    //CRCXORL 33; 
    CRCXORL = 0x21;

    // Read/Write access to CRCOUT
    CRCCON0bits.SETUP = 0b00;
    //CRCOUTT 0; 
    CRCOUTT = 0x0;
    //CRCOUTU 0; 
    CRCOUTU = 0x0;
    //CRCOUTH 0; 
    CRCOUTH = 0x0;
    //CRCOUTL 0; 
    CRCOUTL = 0x0;

    //CRCDATAT 0; 
    CRCDATAT = 0x0;
    //CRCDATAU 0; 
    CRCDATAU = 0x0;
    //CRCDATAH 0; 
    CRCDATAH = 0x0;
    //CRCDATAL 0; 
    CRCDATAL = 0x0;

    //Scanner Configurations
    //
    SCANHADRU = 0x3F;
    //
    SCANHADRH = 0xFF;
    //
    SCANHADRL = 0xFF;
    //LADR undefined; 
    SCANLADRU = 0x0;
    //
    SCANLADRH = 0x0;
    //
    SCANLADRL = 0x0;
    //SCANTSEL LFINTOSC; 
    SCANTRIG = 0x0;

    // Clearing CRC IF flag
    PIR0bits.CRCIF = 0;
    // Disabled CRCI CRC interrupt
    PIE0bits.CRCIE = 0;

    // Clearing Scanner IF flag.
    PIR8bits.SCANIF = 0;
    // Disabled SCANI CRC interrupt
    PIE8bits.SCANIE = 0;

    //CRCEN enabled; CRCGO disabled; CRCACCM data augmented with 0s; CRCSETUP CRC Register Overlay Selection; Read / Write access to CRCOUT; 
    CRCCON0 = 0x90;
    //SCANEN disabled; TRIGEN disabled; SCANGO disabled; MREG Program Flash Memory; BURSTMD CRC request and Trigger; SCANBUSY complete; 
    SCANCON0 = 0x0;
}

inline void CRC_StartCrc(void)
{
    CRCCON0bits.CRCGO = 1;
}

bool CRC_WriteData(uint32_t data)
{
    if(!CRCCON0bits.FULL)
    {
        CRCDATAT = (uint8_t)((data >> 24) & 0xFF);
        CRCDATAU = (uint8_t)((data >> 16) & 0xFF);
        CRCDATAH = (uint8_t)((data >> 8) & 0xFF);
        CRCDATAL = (uint8_t)(data & 0xFF);
        return true;
    } 
    else 
    {
        return false;
    }
}

static uint32_t CRC_ReverseValue(uint32_t crc)
{
    uint32_t mask;
    uint32_t reverse;

    mask = 1;
    mask <<= CRCCON1bits.PLEN;
    reverse = 0;

    while(crc)
    {
        if(crc & 0x01)
        {
            reverse |= mask;
        }
        mask >>= 1;
        crc >>= 1;
    }
    return reverse;
}

uint32_t CRC_GetCalculatedResult(bool reverse, uint32_t xorValue)
{
    uint32_t result = 0x00;
    // Read/Write access to CRCOUT
    CRCCON0bits.SETUP = 0b00;
    result = (uint32_t)CRCOUTL;
    result = result | ((uint32_t)CRCOUTH << 8);
    result = result | ((uint32_t)CRCOUTU << 16);
    result = result | ((uint32_t)CRCOUTT << 24);
    if(reverse)
    {
        result = CRC_ReverseValue(result);
    }
    result ^= xorValue;
    return (result & 0xFFFF);
}

inline bool CRC_IsCrcBusy(void)
{
    return(CRCCON0bits.CRCBUSY);
}

inline void CRC_StartScanner(void)
{
    uint8_t gIntFlagStatus = 0;
    gIntFlagStatus = INTCON0bits.GIE;

    // Disable global Interrupts;
    INTCON0bits.GIE = 0;
    // Grant memory access to CRC Scanner peripherals
    PRLOCK = 0x55;
    PRLOCK = 0xAA;
    PRLOCKbits.PRLOCKED = 1;
    INTCON0bits.GIE = gIntFlagStatus;

    // Start the serial shifter
    CRCCON0bits.CRCGO = 1;
    // Start the scanner
    SCANCON0bits.SGO = 1;
}

inline void CRC_StopScanner(void)
{
    uint8_t gIntFlagStatus = 0;
    gIntFlagStatus = INTCON0bits.GIE;

    // Disable global Interrupts;
    INTCON0bits.GIE = 0;
    // Forbid memory access to CRC Scanner peripherals
    PRLOCK = 0x55;
    PRLOCK = 0xAA;
    PRLOCKbits.PRLOCKED = 0;
    INTCON0bits.GIE = gIntFlagStatus;

    // Stop the serial shifter
    CRCCON0bits.CRCGO = 0;
    // Stop the scanner
    SCANCON0bits.SGO = 0;
}

void CRC_SetScannerAddressLimit(uint24_t startAddr, uint24_t endAddr)
{
    SCANHADRU = (uint8_t)((endAddr >> 16) & 0xFF);
    SCANHADRH = (uint8_t)((endAddr >> 8) & 0xFF);
    SCANHADRL = (uint8_t)(endAddr & 0xFF);
    SCANLADRU = (uint8_t)((startAddr >> 16) & 0xFF);
    SCANLADRH = (uint8_t)((startAddr >> 8) & 0xFF);
    SCANLADRL = (uint8_t)(startAddr & 0xFF);
}

inline bool CRC_IsScannerBusy(void)
{
    return(SCANCON0bits.BUSY);
}


