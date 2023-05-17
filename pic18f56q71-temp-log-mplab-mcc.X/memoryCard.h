#ifndef MEMORYCARD_H
#define	MEMORYCARD_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include <stdbool.h>
        
    typedef enum {
        RESPONSE_R1 = 0x00, RESPONSE_R1B, RESPONSE_R2, RESPONSE_R3
    } RESPONSE_FORMAT;
    
    //Init the Memory Card Driver
    void memCard_init(void);
    
    //Configure the Memory Card
    //Must be called whenever a card is inserted
    bool memCard_initCard(void);
    
    //Reset the memory card
    void memCard_reset(void);
    
    //Send a command to the memory card
    //Command must be in R1 Response Format
    uint8_t memCard_sendCommand_R1(uint8_t commandIndex, uint32_t data);
    
    //Compute CRC7 for the memory card commands
    uint8_t memCard_runCRC7(uint8_t* dataIn, uint8_t len);
    
#ifdef	__cplusplus
}
#endif

#endif	/* MEMORYCARD_H */

