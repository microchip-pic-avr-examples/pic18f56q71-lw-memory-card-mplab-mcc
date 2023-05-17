#include "memoryCard.h"
#include "spi1_host.h"
#include "mcc_generated_files/system/pins.h"

#include <stdint.h>
#include <stdbool.h>

//Init the Memory Card Driver
void memCard_init(void)
{
    
}

//Configure the Memory Card
//Must be called whenever a card is inserted
bool memCard_initCard(void)
{
    return false;
}

//Reset the memory card
void memCard_reset(void)
{
    //Reset the Card
    SPI1_sendResetSequence();
    
    //CMD0 - Reset
    memCard_sendCommand_R1(0x00, 0x00);
}

//Send a command to the memory card
//Command must be in R1 Response Format
uint8_t memCard_sendCommand_R1(uint8_t commandIndex, uint32_t data)
{
    CARD_CS_SetLow();
    uint8_t memPool[6];
    
    //Prepare Command Header
    memPool[0] = 0x40; 
    memPool[0] |= commandIndex;
    
    //Load Data
    memPool[1] = (data & 0xFF000000) >> 24;
    memPool[2] = (data & 0x00FF0000) >> 16;
    memPool[3] = (data & 0x0000FF00) >> 8;
    memPool[4] = (data & 0x000000FF);
    
    //Add the CRC7 Value
    memPool[5] = memCard_runCRC7(&memPool[0], 5);
    
    uint8_t rVal = SPI1_sendCommand_R1(memPool);
    CARD_CS_SetHigh();
    return rVal;
}

//Compute CRC7 for the memory card commands
uint8_t memCard_runCRC7(uint8_t* dataIn, uint8_t len)
{
    uint8_t output = 0x00;
    uint8_t mask;
    //Byte level
    for (uint8_t by = 0; by < len; by++)
    {
        //Bit level
        mask = 0x80;
        while (mask != 0x00)
        {
            //If 1 in the MSB...
            bool input = ((dataIn[by] & mask) != 0x00) ? true : false;
            
            //XOR with the LSB of the 7-bit output
            input ^= (output & 0x01);
            
            //Right-Shift
            output >>= 1;
            
            //Load into shifter
            //Output[7] is not used
            output |= (input << 6);
            
            //Bit 3 = XOR of Prev. and Input
            uint8_t t = output & 0x08;
            uint8_t t2 = (t >> 3) ^ input;
            output = (output & 0xF7) | (t2 << 3);
            
            //Shift mask over by 1
            mask >>= 1;
        }
    }
    
    //Flip the output ordering
    //Note: Bit 7 is not used, and is ignored
    uint8_t tOut;
    tOut = ((output & 0x40) >> 5);
    tOut |= ((output & 0x20) >> 3);
    tOut |= ((output & 0x10) >> 1);
    tOut |= ((output & 0x08) << 1);
    tOut |= ((output & 0x04) << 3);
    tOut |= ((output & 0x02) << 5);
    tOut |= ((output & 0x01) << 7);
    output = tOut;
    
    //Pad with 1
    output |= 0b1;
    
    return output;
}