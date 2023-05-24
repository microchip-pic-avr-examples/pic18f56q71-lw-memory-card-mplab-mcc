#ifndef MEMORYCARD_H
#define	MEMORYCARD_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include <stdbool.h>

#include "mcc_generated_files/system/pins.h"
#include "mcc_generated_files/clc/clc2.h"

//Macro for card insert / detect
#define IS_CARD_ATTACHED() (!CLC2_OutputStatusGet())
    
//Number of bytes to wait for valid response (for R1 commands)
#define TIMEOUT_BYTES 10
    
//How many times will the driver attempt to init the Card (ACMD41 / CMD1)
#define INIT_RETRIES 100
    
//Check Pattern during configuration (Can be any value)
#define CHECK_PATTERN 0xA5
    
//Set VDD for 2.7V to 3.6V Operation
#define VHS_3V3 0b0001
    
//This is returned if no header errors occurred
#define HEADER_IDLE 0x01
    
//This is returned if no errors occurred AFTER init.
#define HEADER_NO_ERROR 0x00
    
//This is returned for an invalid header
#define HEADER_INVALID 0xFF
    
//SPI Baud Rates (Assume SPI Base = 64 MHz)
#define SPI_400KHZ_BAUD 79
#define SPI_10_6MHZ_BAUD 2
    
//Bad OCR return value
#define CARD_BAD_OCR 0xFFFFFFFF
#define CARD_NO_DATA 0x00000000
    
//Number of bytes to transfer
#define FAT_BLOCK_SIZE 512
    
    typedef union 
    {
        struct {
            unsigned is_idle : 1;
            unsigned erase_reset : 1;
            unsigned illegal_cmd_error : 1;
            unsigned crc_error : 1;
            unsigned erase_seq_error : 1;
            unsigned address_error : 1;
            unsigned param_error : 1;
            unsigned valid_header_n : 1; //This value is 0 if the header is valid
        };
        uint8_t data;
    } CommandStatus;
    
    typedef union 
    {
        struct {
            unsigned is_locked : 1;
            unsigned wp_erase_skip : 1;
            unsigned unknown_error : 1;    //Unknown error
            unsigned cc_error : 1; //Controller Error
            unsigned ecc_fail : 1;
            unsigned wp_violation : 1;
            unsigned erase_param : 1;
            unsigned out_of_range : 1;
            unsigned is_idle : 1;
            unsigned erase_reset : 1;
            unsigned illegal_cmd_error : 1;
            unsigned crc_error : 1;
            unsigned erase_seq_error : 1;
            unsigned address_error : 1;
            unsigned param_error : 1;
            unsigned : 1; //Always 0
            
        };
        uint8_t data[2];
        uint16_t packet;
    } CardStatus;
    
    typedef enum {
        CARD_NO_ERROR = 0, CARD_SPI_TIMEOUT, CARD_RESPONSE_ERROR, CARD_ILLEGAL_CMD, CARD_VOLTAGE_NOT_SUPPORTED
    } CommandError;
    
    typedef enum {
        CCS_INVALID = -1, CCS_LOW_CAPACITY, CCS_HIGH_CAPACITY
    } CardCapacityType;
    
    typedef enum {
        STATUS_CARD_NONE = 0, STATUS_CARD_NOT_INIT, STATUS_CARD_ERROR, STATUS_CARD_READY
    } MemoryCardDriverStatus;
    
    //Init the Memory Card Driver
    void memCard_initDriver(void);
    
    //Init an inserted Memory Card
    bool memCard_initCard(void);
    
    //Returns the status of the memory card
    MemoryCardDriverStatus memCard_getCardStatus(void);
    
    //Returns true if the card is ready
    bool memCard_isCardReady(void);
    
    //Notifies the driver that a card is now attached
    //DOES NOT INITIALIZE THE CARD
    void memCard_attach(void);
    
    //Notifies the driver that the card is not attached
    void memCard_detach(void);
    
    //Calls CMD8 to configure the operating voltages
    CommandError memCard_configureCard(void);
    
    //Send a command to the memory card and processes an R1 response
    uint8_t memCard_sendCMD_R1(uint8_t commandIndex, uint32_t data);
    
    //Send an ACOMMAND to the memory card and processes an R1 response
    uint8_t memCard_sendACMD_R1(uint8_t commandIndex, uint32_t data);
    
    //Returns whether the card is high capacity
    CardCapacityType memCard_getCapacityType(void);
    
    //Returns an R1 type response
    bool memCard_receiveResponse_R1(uint8_t* dst);
    
    //Compute CRC7 for the memory card commands
    uint8_t memCard_runCRC7(uint8_t* dataIn, uint8_t len);
    
#ifdef	__cplusplus
}
#endif

#endif	/* MEMORYCARD_H */

