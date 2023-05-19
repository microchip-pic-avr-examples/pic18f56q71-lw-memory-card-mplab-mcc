#include "memoryCard.h"
#include "spi1_host.h"
#include "mcc_generated_files/system/system.h"

#include <stdint.h>
#include <stdbool.h>

volatile bool isCardReady = false;

//Init the Memory Card Driver
void memCard_initDriver(void)
{
    
}

//Configure the Memory Card
//Must be called whenever a card is inserted
bool memCard_initCard(void)
{
    printf("Memory Card Detected\r\n");
    
    //Reset the Card
    SPI1_sendResetSequence();
    
    //CMD0 - Reset
    CommandStatus status;
    status.data = memCard_sendCommand_R1(0x00, 0x00);
    //printf("Return Code: 0x%x\r\n", status.data);
    
    //CMD8
    bool good = memCard_configureCard();
    if (!good)
    {
        printf("CMD8 Failed to Configure Card\r\n");
        return false;
    }    
    
    //ACMD41
    status.data = memCard_sendACommand_R1(41, 0x40000000);
    
    //CMD1
    good = memCard_sendCommand_R1B(1, 0x40000000);
    if (!good)
    {
        printf("CMD1 Failed to Configure Card\r\n");
        return false;
    }
    
    isCardReady = true;
    printf("Memory Card Initialized\r\n");
    return true;
}

//Calls CMD8 to configure the operating voltages
bool memCard_configureCard(void)
{
    CARD_CS_SetLow();
    uint8_t memPoolTx[6];
    uint8_t memPoolRx[6];
    
    //Prepare Command Header
    //0x40 - Fixed + CMD8
    memPoolTx[0] = 0x40 | 8;
    
    //0b0001 - 2.7V to 3.6V operation
    
    //Load Data
    memPoolTx[1] = 0x00;
    memPoolTx[2] = 0x00;
    memPoolTx[3] = VHS_3V3; 
    memPoolTx[4] = CHECK_PATTERN; //Check Pattern
    
    //Add the CRC7 Value
    memPoolTx[5] = memCard_runCRC7(&memPoolTx[0], 5);
    
    //Transmit header
    SPI1_sendBytes(&memPoolTx[0], 6);
    
    bool done = false;
    uint8_t count = 0;
    CommandStatus stat;
    
    if (!memCard_receiveResponse_R1(&stat.data))
    {
        //Response Timeout
        CARD_CS_SetHigh();
        return false;
    }
    
    if (stat.crc_error)
    {
        //Bad CRC - Exit
        CARD_CS_SetHigh();
        return false;
    }
    
    //Now capture 4 more bytes
    
    memPoolTx[0] = 0xFF;
    memPoolTx[1] = 0xFF;
    memPoolTx[2] = 0xFF;
    memPoolTx[3] = 0xFF;
    
    //Get the last bytes of the header
    SPI1_exchangeBytes(&memPoolTx[0], &memPoolRx[0], 4);
    CARD_CS_SetHigh();
    
    //First verify the R1 header
    if (stat.data != 0x01)
    {
        return false;
    }
    
    //Next, verify VHS
    if ((memPoolRx[2] & 0x0F) != VHS_3V3)
    {
        return false;
    }
    
    //Finally, match check pattern
    if (memPoolRx[3] != CHECK_PATTERN)
    {
        return false;
    }
    
    return true;
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
    
    //First transmit the sequence
    SPI1_sendBytes(&memPool[0], 6);
    
    uint8_t rVal = 0xFF;
    
    memCard_receiveResponse_R1(&rVal);
    
    CARD_CS_SetHigh();
    return rVal;
}

//Send a command to the memory card
//Command must be in R1B Response Format
uint8_t memCard_sendCommand_R1B(uint8_t commandIndex, uint32_t data)
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
    
    //First transmit the sequence
    SPI1_sendBytes(&memPool[0], 6);
    
    uint8_t rVal = 0xFF;
    
    memCard_receiveResponse_R1(&rVal);
    
    //TODO: Add Timeout
    
    //Wait for PORTC to go high
    while (!PORTCbits.RC5)
    {
        
    }
    
    CARD_CS_SetHigh();
    return rVal;
}

//Send an ACOMMAND to the memory card
uint8_t memCard_sendACommand_R1(uint8_t commandIndex, uint32_t data)
{
    CommandStatus rVal;
    rVal.data = memCard_sendCommand_R1(55, 0x00);
    
    if (rVal.is_idle)
    {
        
    }
    
    rVal.data = memCard_sendCommand_R1(commandIndex, data);
    
    return rVal.data;
}

//Returns an R1 type response
bool memCard_receiveResponse_R1(uint8_t* dst)
{
    bool done = false;
    uint8_t count = 0;
    CommandStatus stat;
    
    *dst = 0xFF;
    
    while (!done)
    {
        stat.data = SPI1_exchangeByte(0xFF);
        count++;
        if (!stat.valid_header_n)
        {
            //Valid header
            done = true;
        }
        else if (count == TIMEOUT_BYTES)
        {
            return false;
        }
    }
    
    //Load data
    *dst = stat.data;
    return true;
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