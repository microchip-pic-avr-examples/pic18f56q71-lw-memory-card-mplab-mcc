#include "memoryCard.h"
#include "spi1_host.h"
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/timer/delay.h"

#include <stdint.h>
#include <stdbool.h>

#define DEBUG_STRING "[DEBUG] Sending CMD%d\r\n"

static volatile MemoryCardDriverStatus cardStatus = STATUS_CARD_NONE;
static CardCapacityType memCapacity = CCS_INVALID;

static volatile uint8_t cache[512];
static uint32_t cacheBlockAddr;

static uint16_t writeSize;
static bool speedSwitchOK = false;
static uint8_t fastBaud = SPI_400KHZ_BAUD;

void memCard_printData(uint8_t* data, uint8_t size)
{
    for (uint8_t index = 0; index < size; ++index)
    {
        printf("%x%x ", (data[index] & 0xF0) >> 4, data[index] & 0x0F);
    }
    
    printf("\r\n");
}

//Init the Memory Card Driver
void memCard_initDriver(void)
{
    //Clear the Block Address
    cacheBlockAddr = 0xFFFFFFFF;
    writeSize = WRITE_SIZE_INVALID;
    
    if (IS_CARD_ATTACHED())
    {
        cardStatus = STATUS_CARD_NOT_INIT;
    }
    else
    {
        cardStatus = STATUS_CARD_NONE;
    }
}

//Configure the Memory Card
//Must be called whenever a card is inserted
bool memCard_initCard(void)
{
    //If already initialized, skip this step
    if (cardStatus == STATUS_CARD_READY)
    {
        return true;
    }
    
    printf("Beginning memory card configuration...\r\n");
        
    //Invalidate the Cache
    cacheBlockAddr = 0xFFFFFFFF;

    //Invalidate write counter
    writeSize = WRITE_SIZE_INVALID;
    
    //Move to 400 kHz baud to start
    SPI1_setSpeed(SPI_400KHZ_BAUD);
        
    bool good = true;
    
    for (uint8_t fullRetryCount = 0; fullRetryCount < FULL_RETRIES; fullRetryCount++)
    {
        printf("Attempt %d of %d\r\n", (fullRetryCount + 1), FULL_RETRIES);
        
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
            printf("[ERROR] CMD8 failed to configure card ( ");
            switch (err)
            {
                case CARD_NO_ERROR:
                {
                    printf("CARD_NO_ERROR");
                    break;
                }
                case CARD_SPI_TIMEOUT:
                {
                    printf("CARD_SPI_TIMEOUT");
                    break;
                }
                case CARD_CRC_ERROR:
                {
                    printf("CARD_CRC_ERROR");
                    break;
                }
                case CARD_RESPONSE_ERROR:
                {
                    printf("CARD_RESPONSE_ERROR");
                    break;
                }
                case CARD_ILLEGAL_CMD:
                {
                    printf("CARD_ILLEGAL_CMD");
                    break;
                }
                case CARD_VOLTAGE_NOT_SUPPORTED:
                {
                    printf("CARD_VOLTAGE_NOT_SUPPORTED");
                    break;
                }
                case CARD_PATTERN_ERROR:
                {
                    printf("CARD_PATTERN_ERROR");
                    break;
                }
                default:
                    printf("???");
            }
            printf(" )\r\n");
            continue;
        }    
        
        uint8_t count = 1;
        uint32_t initParam = 0x40000000;

        //Reset flag to true
        good = true;
        
        //Try to run ACMD41
        status.data = memCard_sendACMD_R1(41, initParam);

        //Check to see if ACMD41 is accepted
        if (status.illegal_cmd_error)
        {
            //Illegal Command, switch to CMD1
            while ((status.is_idle) && (good))
            {
                //CMD1
                status.data = memCard_sendCMD_R1(1, CARD_NO_DATA);

                if (count >= INIT_RETRIES)
                {
                    printf("[ERROR] CMD1 failed to init card\r\n");
                    good = false;
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
            while ((status.is_idle) && (good))
            {
                //ACMD41
                //0x77 - First Packet
                //0x69 - Second Packet
                status.data = memCard_sendACMD_R1(41, initParam);

                if (count >= INIT_RETRIES)
                {
                    printf("[ERROR] ACMD41 failed to init card\r\n");
                    good = false;
                }
                else
                {
                    DELAY_milliseconds(1);
                    count++;
                }
            }
        }
        
        if (!good)
        {
            //Something went wrong!
            continue;
        }
        
        //Check for High Capacity Support
        //CMD58
        CardCapacityType memCapacity = CCS_INVALID;
        memCapacity = memCard_getCapacityType();
        
#ifdef MEM_CARD_DEBUG_ENABLE
        switch (memCapacity)
        {
            case CCS_LOW_CAPACITY:
                printf("[DEBUG] Memory Card is small - use byte-mode addressing\r\n");
                break;
            case CCS_HIGH_CAPACITY:
                printf("[DEBUG] Memory Card is large - use LBA addressing\r\n");
                break;
            default:
                printf("[WARN] CMD58 was unable to determine capacity support\r\n");
        }
#endif
        
        //Set Block Size to 512B
        //CMD16
        status.data = memCard_sendCMD_R1(16, FAT_BLOCK_SIZE);
        if (status.data != HEADER_NO_ERROR)
        {
            printf("[WARN] Unable to set BLOCK SIZE\r\n");
        }
        
        //Card is now usable for memory operations
        cardStatus = STATUS_CARD_READY;
        printf("Memory Card - READY\r\n");
        
#ifndef DISABLE_SPEED_SWITCH
        //Set SPI Frequency
        if (!memCard_setupFastSPI())
        {
            printf("[WARN] Unable to detect max SPI clock speeds\r\n");
        }
#endif
        
        //Load Block 0 into the cache
        memCard_readBlock(0x00);
        
        return true;
    }
    
    printf("[!] Unable to initialize memory card\r\n");
    cardStatus = STATUS_CARD_ERROR;
    return false;
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

//Requests max clock speed info from card, and sets SPI frequency
bool memCard_setupFastSPI(void)
{
    uint8_t resp[16];
    fastBaud = SPI_400KHZ_BAUD;
    
    //Read the CSD register
    if (memCard_readCSD(&resp[0]) != CARD_NO_ERROR)
    {
        return false;
    }
    
    uint8_t tSpeed = resp[3];
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[DEBUG] Transfer Speed Byte = 0x%x\r\n", tSpeed);
#endif
    
    //Byte 4 contains transfer speed
    //2:0 - Transfer unit
    //6:3 - Multiplier
    //7 - Reserved
    uint8_t tUnit = tSpeed & 0x07;
    
    uint8_t multRef = (tSpeed & 0x78) >> 3;
    if (multRef == 0)
    {
        return false;
    }
    
    if (tUnit > 2)
    {
        //MCU limits the speed, so go-to max speed
        fastBaud = (SPI_10_6MHZ_BAUD);
    }
    else if (tUnit == 2)
    {
        //10 MHz base   
        if (multRef >= 2)
        {
            //Multiplier > 1.1
            fastBaud = (SPI_10_6MHZ_BAUD);
        }
        else
        {
            fastBaud = (SPI_8_MHZ_BAUD);
        }
    }
    else if (tUnit == 1)
    {
        //1 MHz base
        if (multRef == 0x0F)
        {
            fastBaud = (SPI_8_MHZ_BAUD);
        }
        else if (multRef == 0x0E)
        {
            fastBaud = (SPI_6_4MHZ_BAUD);
        }
        else if (multRef >= 9)
        {
            fastBaud = (SPI_4MHZ_BAUD);
        }
        else if (multRef == 8)
        {
            fastBaud = (SPI_3_2MHZ_BAUD);
        }
        else if (multRef >= 5)
        {
            fastBaud = (SPI_2MHZ_BAUD);
        }
        else
        {
            fastBaud = (SPI_1MHZ_BAUD);
        }
    }
    else
    {
        //100 kHz base timing - not worth switching
        return false;
    }
    
    speedSwitchOK = true;
    
    printf("Setting max SPI CLK to ");    
    
    switch(fastBaud)
    {
        case SPI_10_6MHZ_BAUD:
            printf("10.6 MHz\r\n");
            break;
        case SPI_8_MHZ_BAUD:
            printf("8 MHz\r\n");
            break;
        case SPI_6_4MHZ_BAUD:
            printf("6.4 MHz\r\n");
            break;
        case SPI_4MHZ_BAUD:
            printf("4 MHz\r\n");
            break;
        case SPI_3_2MHZ_BAUD:
            printf("3.2 MHz\r\n");
            break;
        case SPI_2MHZ_BAUD:
            printf("2 MHz\r\n");
            break;
        case SPI_1MHZ_BAUD:
            printf("1 MHz\r\n");
            break;
    }
    
    return true;
}

//Calculates the checksum for a block of data
uint16_t memCard_calculateCRC16(uint8_t* data, uint16_t dLen)
{
    uint16_t result = 0x00;
    
    //Setup CRC
    CRCCON0bits.EN = 0;
    CRCCON0bits.SETUP = 0b00;
    
    CRCOUT = 0x00;
    
    CRCCON0bits.EN = 1;
    
    CRC_StartCrc();

    //Calculate
    for (uint16_t i = 0; i < dLen; i++)
    {
        CRC_WriteData(data[i]);
    }

    result = CRC_GetCalculatedResult(false, 0x00) & 0xFFFF;
    return result;
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
    
    //Invalidate the Cache
    cacheBlockAddr = 0xFFFFFFFF;

    //Invalidate write counter
    writeSize = WRITE_SIZE_INVALID;
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
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf(DEBUG_STRING, 8);
#endif
    
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
        return CARD_CRC_ERROR;
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
        return CARD_PATTERN_ERROR;
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
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf(DEBUG_STRING, commandIndex);
#endif
    
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
    uint8_t memPoolRx[4];
    
    if (memCard_readOCR(&memPoolRx[0]) != CARD_NO_ERROR)
    {
        return CCS_INVALID;
    }

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
        else if (count == R1_TIMEOUT_BYTES)
        {
            return false;
        }
    }
    
    //Load data
    *dst = stat.data;
    return true;
}

//Reads the 4-byte OCR Register
CommandError memCard_readOCR(uint8_t* data)
{
    if (cardStatus == STATUS_CARD_NONE)
        return CARD_NOT_INIT;
    
    //Send an extra byte to help the controller between commands
    SPI1_sendByte(0xFF);
    
    uint8_t memPoolTx[6];
    
    //Prepare Command Header
    //0x40 - Fixed + CMD58
    memPoolTx[0] = 0x40 | 58;
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf(DEBUG_STRING, 58);
#endif
    
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
        return CARD_SPI_TIMEOUT;
    }
    
    //Note - card can be idle or init, depending on the reason for reading values
    if ((stat.data & 0xF7) != HEADER_NO_ERROR)
    {
        //Something went wrong
        CARD_CS_SetHigh();
        return CARD_RESPONSE_ERROR;
    }

    //Now capture 4 more bytes
    
    memPoolTx[0] = 0xFF;
    memPoolTx[1] = 0xFF;
    memPoolTx[2] = 0xFF;
    memPoolTx[3] = 0xFF;
    
    //Get the last bytes of the header
    SPI1_exchangeBytes(&memPoolTx[0], &data[0], 4);
    CARD_CS_SetHigh();

#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[DEBUG] Printing OCR Register\r\n");
    memCard_printData(&data[0], 4);
#endif
    
    return CARD_NO_ERROR;
}

//Reads the 16-byte CSD Register
CommandError memCard_readCSD(uint8_t* data)
{
    if (cardStatus != STATUS_CARD_READY)
        return CARD_NOT_INIT;
    
    //Send CSD read command
    //CMD9
    
    uint8_t txData[6];
    uint8_t header;
    
    //Command Header
    txData[0] = 0x40 | 9;
    
    //No arguments
    txData[1] = 0x00;
    txData[2] = 0x00;
    txData[3] = 0x00;
    txData[4] = 0x00;
    
    //Calculate checksum
    txData[5] = memCard_runCRC7(&txData[0], 5);
    
    CARD_CS_SetLow();
    
    //Send Command
    SPI1_sendBytes(&txData[0], 6);
    
    if (!memCard_receiveResponse_R1(&header))
    {
        CARD_CS_SetHigh();
        return CARD_SPI_TIMEOUT;
    }
    
    if (header != HEADER_NO_ERROR)
    {
        //Something went wrong
        CARD_CS_SetHigh();
        return CARD_RESPONSE_ERROR;
    }
    
    CommandError cmdError = memCard_receiveBlockData(&data[0], 16);    
    CARD_CS_SetHigh();
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[DEBUG] Printing CSD Register\r\n");
    memCard_printData(&data[0], 16);
#endif
    
    return cmdError;
}

//Loads data from the memory card into the specified buffer at a block address and byte offset
bool memCard_readFromDisk(uint32_t sect, uint16_t offset, uint8_t* data, uint16_t nBytes)
{
    //Card not initialized
    if (cardStatus != STATUS_CARD_READY)
        return false;
    
#ifdef MEM_CARD_FILE_DEBUG_ENABLE
    printf("[DEBUG FILE I/O] Requesting: Sector %lu at offset %u for %u bytes\r\n", sect, offset, nBytes);
#endif
    
#ifdef MEM_CARD_DISABLE_CACHE
    if (true)
    {
        if (memCard_readBlock(sect) != CARD_NO_ERROR)
        {
            return false;
        }
    }
#else
    if (sect != cacheBlockAddr)
    {
        //Sector not loaded, need to read the value...
        if (memCard_readBlock(sect) != CARD_NO_ERROR)
        {
            return false;
        }
    }
#endif
#ifdef MEM_CARD_FILE_DEBUG_ENABLE
    else
    {
        printf("[DEBUG FILE I/O] Sector cache hit\r\n");
    }
#endif
    
    //Copy data
    uint16_t cachePos = offset;
    for (uint16_t index = 0; index < nBytes; index++)
    {
        if (cachePos == FAT_BLOCK_SIZE)
        {
            //Out of the sector - need to load the next one!
            if (memCard_readBlock(sect + 1) != CARD_NO_ERROR)
            {
                return false;
            }
            cachePos = 0;
        }
#ifdef MEM_CARD_MEMORY_DEBUG_ENABLE
        printf("%x%x ", (cache[cachePos] & 0xF0) >> 4, cache[cachePos] & 0x0F);
#endif
        data[index] = cache[cachePos];
        cachePos++;
    }
    
#ifdef MEM_CARD_MEMORY_DEBUG_ENABLE
            printf("\r\n");
#endif
    
    return true;
}

//Prepare to write to a specified sector.
//Configures write iterators
bool memCard_prepareWrite(uint32_t sector)
{
    if (cardStatus != STATUS_CARD_READY)
    {
        return false;
    }
    
#ifdef MEM_CARD_FILE_DEBUG_ENABLE
    printf("[DEBUG FILE I/O] Preparing for write on sector %lu\r\n", sector);
#endif
    
    //Set the target
    cacheBlockAddr = sector;
    
    //Set the write value
    writeSize = 0;
    
    for (uint16_t i = 0; i < FAT_BLOCK_SIZE; i++)
    {
        cache[i] = 0x00;
    }
    
    return true;
}

//Queues dLen bytes of data to write, sets bw to the number of bytes queued
//Returns true if successful, false if failed
bool memCard_queueWrite(uint8_t* data, uint16_t dLen)
{   
    //Card isn't ready
    if (cardStatus != STATUS_CARD_READY)
    {
        return false;
    }

    //Check write counter
    if ((writeSize == WRITE_SIZE_INVALID) || (writeSize >= FAT_BLOCK_SIZE))
    {
        return false;
    }
    
    uint16_t count = 0;
    while ((writeSize < FAT_BLOCK_SIZE) && (count < dLen))
    {
        cache[writeSize] = data[count];
        
        writeSize++;
        count++;
    }
    
#ifdef MEM_CARD_FILE_DEBUG_ENABLE
    printf("[DEBUG FILE I/O] Queued %u bytes for write\r\n", count);
#endif
    
    if (count != dLen)
    {
        //Out of space!
        return false;
    }
    
    return true;
}

//Writes the current (modified) cache to the memory card
CommandError memCard_writeBlock(void)
{
    if (cardStatus != STATUS_CARD_READY)
    {
        return CARD_NOT_INIT;
    }
    
    if ((writeSize == WRITE_SIZE_INVALID) || (writeSize >= FAT_BLOCK_SIZE))
    {
        return CARD_WRITE_SIZE_ERROR;
    }
    
#ifdef MEM_CARD_FILE_DEBUG_ENABLE
    printf("[DEBUG FILE I/O] Writing %u bytes to sector %lu \r\n", writeSize, cacheBlockAddr);
#endif
    
    uint32_t compBlockAddr = cacheBlockAddr;
    if (memCapacity != CCS_HIGH_CAPACITY)
    {
        //Shift by 9 bits (512) to convert block to byte addressing
        compBlockAddr <<= FAT_BLOCK_SHIFT;
    }
    
    //Send CMD24
    uint8_t cmdData[6];
    cmdData[0] = 0x40 | 24;
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf(DEBUG_STRING, 24);
#endif
    
    //Pack the address
    cmdData[1] = (compBlockAddr & 0xFF000000) >> 24;
    cmdData[2] = (compBlockAddr & 0x00FF0000) >> 16;
    cmdData[3] = (compBlockAddr & 0x0000FF00) >> 8;
    cmdData[4] = (compBlockAddr & 0x000000FF);
    
    cmdData[5] = memCard_runCRC7(&cmdData[0], 5);
    
    CARD_CS_SetLow();
    
    //Send CMD
    SPI1_sendBytes(&cmdData[0], 6);
    
    //Get the Response
    CommandStatus header;
    if (!memCard_receiveResponse_R1(&header.data))
    {
        CARD_CS_SetHigh();
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[ERROR] No response returned\r\n");
#endif

        return CARD_SPI_TIMEOUT;
    }
    
    if (header.data != 0x00)
    {
        CARD_CS_SetHigh();
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[ERROR] Command response error\r\n");
#endif
        return CARD_RESPONSE_ERROR;
    }
    
    //Clock Speed Switching
#ifndef DISABLE_SPEED_SWITCH
    if (speedSwitchOK)
    {
        SPI1_setSpeed(fastBaud);
    }
#endif
    
    //Padding Byte
    //Do we need this?
    //SPI1_sendByte(0xFF);

    //Send Data Packet
    
    //Header Byte
    SPI1_sendByte(0xFE);
    
    //Send Data!
    SPI1_sendBytes(&cache[0], FAT_BLOCK_SIZE);
      
    uint16_t chkSum = memCard_calculateCRC16(&cache[0], FAT_BLOCK_SIZE);
    
    //CRC (Usually ignored...)
    SPI1_sendByte(((chkSum >> 8) & 0xFF));
    SPI1_sendByte(chkSum & 0xFF);
    
    //Receive Data Response
    RespToken eToken;
    uint8_t rCount = 0;
    bool good = false;
    
    //Return to 400 kHz base
    SPI1_setSpeed(SPI_400KHZ_BAUD);
    
    do 
    {
        eToken.data = SPI1_exchangeByte(0xFF);
        
        //Valid header!
        if (eToken.data != 0xFF)
        {
            good = true;
        }
        
        rCount++;
    } while ((rCount < WRITE_TIMEOUT_BYTES) && (!good));
    
    if (rCount >= WRITE_TIMEOUT_BYTES)
    {
        CARD_CS_SetHigh();
        return CARD_SPI_TIMEOUT;
    }
    
    //Type - Error Token
    if (eToken.ErrorToken.valid_header_n == 0b000)
    {
        if (eToken.data != 0x00)
        {
            //Error returned!
            CARD_CS_SetHigh();
            return CARD_RESPONSE_ERROR;
        }
    }
    else
    {
        //Type - Data Token
        if (eToken.DataToken.status != 0b010)
        {
            //Error returned!
            CARD_CS_SetHigh();
            return CARD_RESPONSE_ERROR;
        }
    }
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[DEBUG] Waiting for busy to clear...\r\n");
#endif
    
    //Wait for busy to clear...
    //Active LOW
    
    uint8_t resp;
    do 
    {
        resp = SPI1_exchangeByte(0xFF);
    }while (resp == 0x00);
    
    CARD_CS_SetHigh();
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[DEBUG] Busy bit has cleared - write done!\r\n");
#endif
    
    writeSize = WRITE_SIZE_INVALID;
    cacheBlockAddr = 0xFFFFFFFF;
    
    return CARD_NO_ERROR;
}

//Reads a block of data, and loads it into cache
CommandError memCard_readBlock(uint32_t blockAddr)
{
    if (cardStatus != STATUS_CARD_READY)
    {
        return CARD_NOT_INIT;
    }
    
    if (writeSize != WRITE_SIZE_INVALID)
    {
#ifdef MEM_CARD_FILE_DEBUG_ENABLE
    printf("[DEBUG FILE I/O] Read failed due to write in progress\r\n");
#endif
        return CARD_WRITE_IN_PROGRESS;
    }
    
#ifndef MEM_CARD_DISABLE_CACHE
    if (blockAddr == cacheBlockAddr)
    {
#ifdef MEM_CARD_FILE_DEBUG_ENABLE
    printf("[DEBUG FILE I/O] Sector %lu fetch skipped due to cache\r\n", blockAddr);
#endif
        return CARD_NO_ERROR;
    }
#endif
    
#ifdef MEM_CARD_FILE_DEBUG_ENABLE
    printf("[DEBUG FILE I/O] Fetching Sector %lu\r\n", blockAddr);
#endif
    
    uint32_t compBlockAddr = blockAddr;
    
    if (memCapacity != CCS_HIGH_CAPACITY)
    {
        //Shift by 9 bits (512) to convert block to byte addressing
        compBlockAddr <<= FAT_BLOCK_SHIFT;
    }
    
    //Send CMD17
    uint8_t cmdData[6];
    cmdData[0] = 0x40 | 17;
    
#ifdef MEM_CARD_DEBUG_ENABLE
    printf(DEBUG_STRING, 17);
#endif
    
    //Pack the address
    cmdData[1] = (compBlockAddr & 0xFF000000) >> 24;
    cmdData[2] = (compBlockAddr & 0x00FF0000) >> 16;
    cmdData[3] = (compBlockAddr & 0x0000FF00) >> 8;
    cmdData[4] = (compBlockAddr & 0x000000FF);
    
    //Calculate CRC7
    cmdData[5] = memCard_runCRC7(&cmdData[0], 5);
    
    CARD_CS_SetLow();
    
    //Send CMD
    SPI1_sendBytes(&cmdData[0], 6);
    
    uint8_t header;
    if (!memCard_receiveResponse_R1(&header))
    {
        CARD_CS_SetHigh();
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[ERROR] No response returned\r\n");
#endif

        return CARD_SPI_TIMEOUT;
    }
    
    if (header != HEADER_NO_ERROR)
    {
        //Something went wrong
        CARD_CS_SetHigh();
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[ERROR] Command Error\r\n");
#endif

        
        return CARD_RESPONSE_ERROR;
    }
    
    //Receive data
    CommandError err = memCard_receiveBlockData(&cache[0], FAT_BLOCK_SIZE);
        
    CARD_CS_SetHigh();
    
    //Update Cache Address
    cacheBlockAddr = blockAddr;
        
    return err;
}

//Receives length bytes of data. Does not transmit the command
CommandError memCard_receiveBlockData(uint8_t* data, uint16_t length)
{    
    //Data Header
    RespToken eToken;
    eToken.data = 0xFF;
    uint8_t rCount = 0;
    bool good = false;

    do 
    {
        eToken.data = SPI1_exchangeByte(0xFF);
        
        //Valid header!
        if (eToken.data != 0xFF)
        {
            good = true;
        }
        
        rCount++;
    } while ((rCount < READ_TIMEOUT_BYTES) && (!good));
    
    if (rCount >= READ_TIMEOUT_BYTES)
    {
        return CARD_SPI_TIMEOUT;
    }
    
    if (eToken.data != 0xFE)
    {
        //Error returned!
        return CARD_RESPONSE_ERROR;
    }
    
    //Clock Speed Switching
#ifndef DISABLE_SPEED_SWITCH
    if (speedSwitchOK)
    {
        SPI1_setSpeed(fastBaud);
    }
#endif
    
    //Receive Data
    SPI1_receiveBytesTransmitFF(&data[0], length);
    
    uint8_t crcResp[2];
    
    //Finally, get 2 bytes for checksum
    SPI1_receiveBytesTransmitFF(&crcResp[0], 2);
    
    CARD_CS_SetHigh();
    SPI1_setSpeed(SPI_400KHZ_BAUD);
    
    //CRC16 CCIT Polynomial
    //0x1021
    
#ifdef CRC_VALIDATE_READ
    
#ifdef MEM_CARD_MEMORY_DEBUG_ENABLE
    printf("[DEBUG] Data CRC = ");
    memCard_printData(&crcResp[0], 2);
#endif
    
    uint16_t crcOut;
    
    //Flush CRC Buffer before beginning
    CRCCON0bits.SETUP = 0b00;
    CRCOUT = 0x00000000;
    
    for (uint16_t i = 0; i < length; ++i)
    {
        CRC_WriteData(data[i]);
        while (CRC_IsCrcBusy());
    }
    
    //Now, load the CRC checksum in
    CRC_WriteData(crcResp[0]);
    while (CRC_IsCrcBusy());
    CRC_WriteData(crcResp[1]);
    while (CRC_IsCrcBusy());

    crcOut = CRC_GetCalculatedResult(false, 0x00);
            
    //CRC Failed
    if (crcOut != 0x0000)
    {
        printf("CRC failed during read\r\nC");
#ifdef ENFORCE_DATA_CRC 
        cacheBlockAddr = 0xFFFFFFFF;
        return CARD_CRC_ERROR;
#endif
    }
    
#endif
    return CARD_NO_ERROR;
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