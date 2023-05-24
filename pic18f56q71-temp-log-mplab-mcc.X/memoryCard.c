#include "memoryCard.h"
#include "spi1_host.h"
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/timer/delay.h"

#include <stdint.h>
#include <stdbool.h>

volatile MemoryCardDriverStatus cardStatus = STATUS_CARD_NONE;

//Init the Memory Card Driver
void memCard_initDriver(void)
{
    if (IS_CARD_ATTACHED())
    {
        cardStatus = STATUS_CARD_NOT_INIT;
    }
}

//Configure the Memory Card
//Must be called whenever a card is inserted
bool memCard_initCard(void)
{
    printf("Memory card detected\r\n");
    
    //Move to 400 kHz baud to start
    SPI1_setSpeed(SPI_400KHZ_BAUD);
    
    //Reset the Card
    SPI1_sendResetSequence();
    
    //CMD0 - Reset
    CommandStatus status;
    status.data = memCard_sendCMD_R1(0x00, CARD_NO_DATA);
    //printf("Return Code: 0x%x\r\n", status.data);
    
    //CMD8
    CommandError err = memCard_configureCard();
    if (err != CARD_NO_ERROR)
    {
        printf("CMD8 failed to configure card\r\n");
        cardStatus = STATUS_CARD_ERROR;
        return false;
    }    
        
    //Check for High Capacity Support
    //CMD58
    CardCapacityType memCapacity = CCS_INVALID;
    memCapacity = memCard_getCapacityType();
    
    switch (memCapacity)
    {
        case CCS_LOW_CAPACITY:
        {
            printf("Card is not high capacity type\r\n");
            break;
        }
        case CCS_HIGH_CAPACITY:
        {
            printf("Card is high capacity type\r\n");
            break;
        }
        default:
        {
            printf("CMD58 was unable to determine capacity support\r\n");
        }
    }

    uint8_t count = 1;
    uint32_t initParam = 0x40000000;
    
    //Try to run ACMD41
    status.data = memCard_sendACMD_R1(41, initParam);
    
    //Check to see if ACMD41 is accepted
    if (status.illegal_cmd_error)
    {
        //Illegal Command, switch to CMD1
        while (status.is_idle)
        {
            //CMD1
            status.data = memCard_sendCMD_R1(1, CARD_NO_DATA);
            
            if (count >= INIT_RETRIES)
            {
                printf("CMD1 failed to init card\r\n");
                cardStatus = STATUS_CARD_ERROR;
                return false;
            }
            else
            {
                DELAY_milliseconds(1);
                count++;
            }
        }
    }
    else
    {
        //Valid Command
        while (status.is_idle)
        {
            //ACMD41
            //0x77 - First Packet
            //0x69 - Second Packet
            status.data = memCard_sendACMD_R1(41, initParam);
            
            if (count >= INIT_RETRIES)
            {
                printf("ACMD41 failed to init card\r\n");
                cardStatus = STATUS_CARD_ERROR;
                return false;
            }
            else
            {
                DELAY_milliseconds(1);
                count++;
            }
        }
    }
    
    //Set Block Size to 512B
    //CMD16
    if (memCapacity != CCS_HIGH_CAPACITY)
    {
        status.data = memCard_sendCMD_R1(16, FAT_BLOCK_SIZE);
        if (status.data != HEADER_NO_ERROR)
        {
            printf("Unable to set BLOCK SIZE\r\n");
        }
    }
    
    cardStatus = STATUS_CARD_READY;
    printf("Memory card initialized\r\n");
    return true;
}

//Returns the status of the memory card
MemoryCardDriverStatus memCard_getCardStatus(void)
{
    return cardStatus;
}

//Returns true if the card is ready
bool memCard_isCardReady(void)
{
    return (cardStatus == STATUS_CARD_READY);
}

//Notifies the driver that a card is now attached
//DOES NOT INITIALIZE THE CARD
void memCard_attach(void)
{
    cardStatus = STATUS_CARD_NOT_INIT;
}

//Notifies the driver that the card is not attached
void memCard_detach(void)
{
    cardStatus = STATUS_CARD_NONE;
}

//Calls CMD8 to configure the operating voltages
CommandError memCard_configureCard(void)
{
    //Send an extra byte to help the controller between commands
    SPI1_sendByte(0xFF);
    
    uint8_t memPoolTx[6];
    uint8_t memPoolRx[6];
    
    //Prepare Command Header
    //0x40 - Fixed + CMD8
    memPoolTx[0] = 0x40 | 8;
    
    //Load Data
    memPoolTx[1] = 0x00;
    memPoolTx[2] = 0x00;
    memPoolTx[3] = VHS_3V3; 
    memPoolTx[4] = CHECK_PATTERN; //Check Pattern
    
    //Add the CRC7 Value
    memPoolTx[5] = memCard_runCRC7(&memPoolTx[0], 5);
    
    CARD_CS_SetLow();
    
    //Transmit header
    SPI1_sendBytes(&memPoolTx[0], 6);
    
    CommandStatus stat;
    
    if (!memCard_receiveResponse_R1(&stat.data))
    {
        //Response Timeout
        CARD_CS_SetHigh();
        return CARD_SPI_TIMEOUT;
    }
    
    if (stat.illegal_cmd_error)
    {
        //Illegal Command
        CARD_CS_SetHigh();
        return CARD_ILLEGAL_CMD;
    }
    
    if (stat.crc_error)
    {
        //Bad CRC - Exit
        CARD_CS_SetHigh();
        return CARD_RESPONSE_ERROR;
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
    if (stat.data != HEADER_IDLE)
    {
        return CARD_RESPONSE_ERROR;
    }
    
    //Next, verify VHS
    if ((memPoolRx[2] & 0x0F) != VHS_3V3)
    {
        return CARD_VOLTAGE_NOT_SUPPORTED;
    }
    
    //Finally, match check pattern
    if (memPoolRx[3] != CHECK_PATTERN)
    {
        return CARD_RESPONSE_ERROR;
    }
    
    return CARD_NO_ERROR;
}

//Send a command to the memory card
//Command must be in R1 Response Format
uint8_t memCard_sendCMD_R1(uint8_t commandIndex, uint32_t data)
{
    //Send an extra byte to help the controller between commands
    SPI1_sendByte(0xFF);
    
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
    
    CARD_CS_SetLow();
    
    //First transmit the sequence
    SPI1_sendBytes(&memPool[0], 6);
    
    uint8_t rVal = 0xFF;
    
    memCard_receiveResponse_R1(&rVal);
    
    CARD_CS_SetHigh();
    return rVal;
}

//Send an ACOMMAND to the memory card
uint8_t memCard_sendACMD_R1(uint8_t commandIndex, uint32_t data)
{
    CommandStatus rVal;
    rVal.data = memCard_sendCMD_R1(55, CARD_NO_DATA);
    
    if (rVal.data != HEADER_IDLE)
    {
        return HEADER_INVALID;
    }
    
    rVal.data = memCard_sendCMD_R1(commandIndex, data);
    
    return rVal.data;
}

//Returns the OCR register from the memory card
CardCapacityType memCard_getCapacityType(void)
{
    //Send an extra byte to help the controller between commands
    SPI1_sendByte(0xFF);
    
    uint8_t memPoolTx[6];
    uint8_t memPoolRx[6];
    
    //Prepare Command Header
    //0x40 - Fixed + CMD58
    memPoolTx[0] = 0x40 | 8;
    
    //No data!
    memPoolTx[1] = 0x00;
    memPoolTx[2] = 0x00;
    memPoolTx[3] = 0x00; 
    memPoolTx[4] = 0x00;
    
    //Compute the CRC7 Value
    memPoolTx[5] = memCard_runCRC7(&memPoolTx[0], 5);
    
    CARD_CS_SetLow();
    
    //Transmit header
    SPI1_sendBytes(&memPoolTx[0], 6);
    
    CommandStatus stat;
    
    if (!memCard_receiveResponse_R1(&stat.data))
    {
        //Response Timeout
        CARD_CS_SetHigh();
        return CCS_INVALID;
    }
    
    if (stat.data != HEADER_IDLE)
    {
        //Something went wrong
        CARD_CS_SetHigh();
        return CCS_INVALID;
    }

    //Now capture 4 more bytes
    
    memPoolTx[0] = 0xFF;
    memPoolTx[1] = 0xFF;
    memPoolTx[2] = 0xFF;
    memPoolTx[3] = 0xFF;
    
    //Get the last bytes of the header
    SPI1_exchangeBytes(&memPoolTx[0], &memPoolRx[0], 4);
    CARD_CS_SetHigh();

    //OCR is bit 30 from the return
    return ((memPoolRx[0] & 0x40) != 0x00) ? CCS_HIGH_CAPACITY : CCS_LOW_CAPACITY;
}

//Returns an R1 type response
bool memCard_receiveResponse_R1(uint8_t* dst)
{
    bool done = false;
    uint8_t count = 0;
    CommandStatus stat;
    
    *dst = HEADER_INVALID;
    
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