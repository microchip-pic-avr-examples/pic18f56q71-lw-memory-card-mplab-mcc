/*
© [2022] Microchip Technology Inc. and its subsidiaries.
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

#ifndef SPI1_HOST_H
#define	SPI1_HOST_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>
    
    //Initializes a SPI Host at 400 kHz
    //I/O must be initialized separately
    void SPI1_initHost(void);
    
    //Initializes the I/O for the SPI Host
    void SPI1_initPins(void);
    
    //Sets the clock speed of SPI1
    //F_SPI = Fclk / (2 * (BAUD + 1))
    void SPI1_setSpeed(uint8_t baud);
    
    //Sends and receives a single byte
    uint8_t SPI1_exchangeByte(uint8_t data);
    
    //Sends a single byte. Received data is discarded
    void SPI1_sendByte(uint8_t data);
    
    //Receives a single byte
    uint8_t SPI1_recieveByte(void);
    
    //Send and receives LEN bytes
    void SPI1_exchangeBytes(uint8_t* txData, uint8_t* rxData, uint8_t len);
    
    //Sends LEN bytes. Received data is discarded
    void SPI1_sendBytes(uint8_t* txData, uint16_t len);
    
    //Transmit LEN zeros
    void SPI1_fillZeros(uint16_t len);
    
    //Receives LEN bytes
    void SPI1_receiveBytes(uint8_t* rxData, uint8_t len);
    
    //Receives LEN bytes, and transmits 0xFF
    void SPI1_receiveBytesTransmitFF(uint8_t* rxData, uint16_t len);
    
    //Transmits a 6 byte header, then returns the next byte after
    uint8_t SPI1_sendCommand_R1(uint8_t* data);
    
    //Transmits a 6 byte header, then returns a 5 byte header
    void SPI1_sendCommand_R7(uint8_t* txData, uint8_t* rxData);
    
    //Sends 10 bytes (80 bits) worth of clock cycles for the memory card to boot
    void SPI1_sendResetSequence(void);
    
#ifdef	__cplusplus
}
#endif

#endif	/* SPI_HOST_H */

