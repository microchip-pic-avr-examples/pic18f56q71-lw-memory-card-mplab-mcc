#include "spi1_host.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

//Initializes a SPI Host
//I/O must be initialized separately
void SPI1_initHost(void)
{
    //Host Mode, Bit Mode
    SPI1CON0 = 0x00;
    SPI1CON0bits.MST = 1;
    
    //Data shifted on falling edge, SS is Active Low
    SPI1CON1 = 0x00;
    SPI1CON1bits.CKE = 1;
    SPI1CON1bits.SSP = 1;
    
    //Clear RXR, TXR, SS is active only when CNT > 0
    SPI1CON2 = 0x00;
    
    //Select HFINTOSC as Clock Source
    SPI1CLK = 0b00001;
    
    //From a 64MHz clock, 400 kHz SCK
    SPI1BAUD = 79;
    
    //Set Width to 8-bits (n = 0)
    SPI1TWIDTH = 0;
    
    //Enable SPI
    SPI1CON0bits.EN = 1;
}

//Initializes the I/O for the SPI Host
void SPI1_initPins(void)
{
    //RC2 - SDO
    //RC5 - SDI
    //RC6 - SCK
    //RA5 - SS1 (alt. CS1)
    
    //SDO Config
    TRISC2 = 0;
    RC2PPS = 0x1E;
    SLRCONCbits.SLRC2 = 0;
    
    //SDI Config
    TRISC5 = 1;
    ANSELC5 = 0;
    SPI1SDIPPS = 0b010101;
    SLRCONCbits.SLRC5 = 0;
    
    //SCK Config
    TRISC6 = 0;
    RC6PPS = 0x1D;
    SLRCONCbits.SLRC6 = 0;
    
#ifdef HW_SS_ENABLE
    //CS Config
    TRISA5 = 0;
    RA5PPS = 0x1F;
#endif
}

//Sets the clock speed of SPI1    
void SPI1_setSpeed(uint8_t baud)
{
    SPI1CON0bits.EN = 0;
    SPI1BAUD = baud;
    SPI1CON0bits.EN = 1;
}

//Sends and receives a single byte
uint8_t SPI1_exchangeByte(uint8_t data)
{
    uint8_t output = data;
    SPI1_exchangeBytes(&output, &output, 1);
    return output;
}

//Sends a single byte. Received data is discarded.
void SPI1_sendByte(uint8_t data)
{
    SPI1_sendBytes(&data, 1);
}

//Receives a single byte. Transmitted data is 0x00
uint8_t SPI1_recieveByte(void)
{
    uint8_t rx = 0x00;
    SPI1_receiveBytes(&rx, 1);
    return rx;
}

//Send and receives LEN bytes.
void SPI1_exchangeBytes(uint8_t* txData, uint8_t* rxData, uint8_t len)
{
    //Clear data buffers
    SPI1STATUSbits.CLRBF = 1;
    
    //Enable TX and RX
    SPI1CON2bits.TXR = 1;
    SPI1CON2bits.RXR = 1;
    
    //Clear status bit
    SPI1INTFbits.TCZIF = 0;
    
    //Load Byte 0
    SPI1TXB = txData[0];
    
    //Set data length
    SPI1TCNTL = len;
    
    //Write / Read Index
    uint8_t wIndex = 1, rIndex = 0;
    
    //While counter is not zero
    while (!SPI1INTFbits.TCZIF)
    {
        if ((PIR3bits.SPI1TXIF) && (wIndex < len))
        {
            //TX Buffer has space, load next byte (until we hit the LEN)
            SPI1TXB = txData[wIndex];
            wIndex++;
        }
        
        if (PIR3bits.SPI1RXIF)
        {
            //RX Buffer Ready
            rxData[rIndex] = SPI1RXB;
            rIndex++;
        }
    }
    
    //Protects against a possible edge case where a byte is received as the module stops
    if (PIR3bits.SPI1RXIF)
    {
        //RX Buffer Ready
        rxData[rIndex] = SPI1RXB;
        rIndex++;
    }
}

//Sends LEN bytes. Received data is discarded.
void SPI1_sendBytes(uint8_t* txData, uint16_t len)
{
    //Clear data buffers
    SPI1STATUSbits.CLRBF = 1;
    
    //Enable TX and Disable RX
    SPI1CON2bits.TXR = 1;
    SPI1CON2bits.RXR = 0;
    
    //Clear status bit
    SPI1INTFbits.TCZIF = 0;
    
    //Load Byte 0
    SPI1TXB = txData[0];
    
    //Set data length
    SPI1TCNTH = (len >> 8) & 0xFF;
    SPI1TCNTL = len & 0xFF;
    
    //Write / Read Index
    uint16_t wIndex = 1;
    
    //While counter is not zero
    while (!SPI1INTFbits.TCZIF)
    {
        if ((PIR3bits.SPI1TXIF) && (wIndex < len))
        {
            //TX Buffer has space, load next byte (until we hit the LEN)
            SPI1TXB = txData[wIndex];
            wIndex++;
        }
    }
}

//Transmit LEN zeros
void SPI1_fillZeros(uint16_t len)
{
    //Clear data buffers
    SPI1STATUSbits.CLRBF = 1;
    
    //Enable TX and Disable RX
    SPI1CON2bits.TXR = 1;
    SPI1CON2bits.RXR = 0;
    
    //Clear status bit
    SPI1INTFbits.TCZIF = 0;
    
    //Load Byte 0
    SPI1TXB = 0x00;
    
    //Set data length
    SPI1TCNTH = (len >> 8) & 0xFF;
    SPI1TCNTL = len & 0xFF;
    
    //Write / Read Index
    uint16_t wIndex = 1;
    
    //While counter is not zero
    while (!SPI1INTFbits.TCZIF)
    {
        if ((PIR3bits.SPI1TXIF) && (wIndex < len))
        {
            //TX Buffer has space, load next byte (until we hit the LEN)
            SPI1TXB = 0x00;
            wIndex++;
        }
    }
}

//Receives LEN bytes. Transmitted data is 0x00
void SPI1_receiveBytes(uint8_t* rxData, uint8_t len)
{
    //Clear data buffers
    SPI1STATUSbits.CLRBF = 1;
    
    //Enable RX and Disable TX
    SPI1CON2bits.TXR = 0;
    SPI1CON2bits.RXR = 1;
    
    //Clear status bit
    SPI1INTFbits.TCZIF = 0;
    
    //Set data length
    SPI1TCNTL = len;
    
    //Write / Read Index
    uint8_t rIndex = 0;
    
    //While counter is not zero
    while (!SPI1INTFbits.TCZIF)
    {
        //Protects against a possible edge case where a byte is received as the module stops
        if (PIR3bits.SPI1RXIF)
        {
            //RX Buffer Ready
            rxData[rIndex] = SPI1RXB;
            rIndex++;
        }
    }
    
    //Protects against a possible edge case where a byte is received as the module stops
    if (PIR3bits.SPI1RXIF)
    {
        //RX Buffer Ready
        rxData[rIndex] = SPI1RXB;
        rIndex++;
    }
}

//Receives LEN bytes, and transmits 0xFF
void SPI1_receiveBytesTransmitFF(uint8_t* rxData, uint16_t len)
{
    //Clear data buffers
    SPI1STATUSbits.CLRBF = 1;
    
    //Enable RX and TX
    SPI1CON2bits.TXR = 1;
    SPI1CON2bits.RXR = 1;
    
    //Clear status bit
    SPI1INTFbits.TCZIF = 0;
    
    //Set data length
    SPI1TCNTH = (len >> 8) & 0xFF;
    SPI1TCNTL = len & 0xFF;
    
    SPI1TXB = 0xFF;
    
    //Write / Read Index
    uint16_t rIndex = 0;
    uint16_t wCount = 1;
    
    //While counter is not zero
    while (!SPI1INTFbits.TCZIF)
    {
        if ((PIR3bits.SPI1TXIF) && (wCount < len))
        {
            //TX Buffer has space, load next byte
            SPI1TXB = 0xFF;
            wCount++;
        }
        
        //Protects against a possible edge case where a byte is received as the module stops
        if (PIR3bits.SPI1RXIF)
        {
            //RX Buffer Ready
            rxData[rIndex] = SPI1RXB;
            rIndex++;
        }
    }
    
    //Protects against a possible edge case where a byte is received as the module stops
    if (PIR3bits.SPI1RXIF)
    {
        //RX Buffer Ready
        rxData[rIndex] = SPI1RXB;
        rIndex++;
    }
}

//Sends 10 bytes (80 bits) worth of clock cycles for the memory card to boot
void SPI1_sendResetSequence(void)
{
    //Clear data buffers
    SPI1STATUSbits.CLRBF = 1;
    
    //Enable TX and Disable RX
    SPI1CON2bits.TXR = 1;
    SPI1CON2bits.RXR = 0;
    
    //Clear status bit
    SPI1INTFbits.TCZIF = 0;
    
    //Load Byte 0
    SPI1TXB = 0xFF;
    
    //Set data length
    SPI1TCNTL = 10;
    
    //Write / Read Index
    uint8_t wIndex = 1;
    
    //While counter is not zero
    while (!SPI1INTFbits.TCZIF)
    {
        if ((PIR3bits.SPI1TXIF) && (wIndex < 10))
        {
            //TX Buffer has space, load next byte (until we hit the LEN)
            SPI1TXB = 0xFF;
            wIndex++;
        }
    }

}