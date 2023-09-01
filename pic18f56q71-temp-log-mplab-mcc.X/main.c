 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.0
*/

/*
© [2023] Microchip Technology Inc. and its subsidiaries.

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
#include "mcc_generated_files/system/system.h"
#include "spi1_host.h"
#include "memoryCard.h"
#include "unitTests.h"
#include "Petite-FatFs/diskio.h"
#include "Petite-FatFs/pff.h"

#include <stdint.h>
#include <stdbool.h>

#include <assert.h>
#define STOP() do{__conditional_software_breakpoint(0);NOP();}while(0)
#define ASSERT(x) do{__conditional_software_breakpoint(x);NOP();}while(0)

//#define UNIT_TEST_ENABLE

void onCardChange(void)
{
    if (IS_CARD_ATTACHED())
    {
        memCard_attach();
    }
    else
    {
        memCard_detach();
    }
}

uint16_t getStringLength(const char* str)
{
    uint16_t count = 0;
    while (str[count] != '\0') { count++; }
    
    //Add an extra position to account for '\0'
    count++;
    return count;
}

void modifyFile(const char* filename)
{
    FRESULT result;
    unsigned int rxLen = 0;
    char rxBuffer[127];
    
    const char* newMessage = "Hello from PIC18F56Q71";
    unsigned int bwLen = 0;
    
    printf("Reading file \"%s\"\r\n", filename);
    
    //Open a file
    result = pf_open(filename);
    if (result == FR_OK)
    {
        //File opened OK
        
        //Read the original Message
        if (pf_read(&rxBuffer[0], 126, &rxLen) == FR_OK)
        {
            rxBuffer[rxLen] = '\0';
            printf("Printing file \"%s\"\r\n> %s\r\n", filename, rxBuffer);
        }
        else
        {
            printf("[ERROR] Failed to read file\r\n");
            return;
        }
        
        //Overwrite the original message
        //First queue the new text
        result = pf_write(&newMessage[0], getStringLength(newMessage), &bwLen);
        if (result == FR_OK)
        {
            //Then, commit it to the card
            result = pf_write(0,0, &bwLen);
            if (result == FR_OK)
            {
                printf("File was successfully modified\r\n");
            }
            else
            {
                printf("[ERROR] Failed to write file\r\n");
                return;
            }
        }
        else
        {
            printf("[ERROR] Failed to queue write data\r\n");
            return;
        }
        
        //Move read/write pointer
        result = pf_lseek(0);
        if (result == FR_OK)
        {
            printf("Returning to start of file\r\n");
        }
        else
        {
            printf("[ERROR] Failed to seek file\r\n");
            return;
        }
        
        //Read the new message
        if (pf_read(&rxBuffer[0], 126, &rxLen) == FR_OK)
        {
            rxBuffer[rxLen] = '\0';
            printf("Printing modified file \"%s\"\r\n> %s\r\n", filename, rxBuffer);
        }
        else
        {
            printf("[ERROR] Failed to read file\r\n");
            return;
        }
    }
    else
    {
        printf("[ERROR] Could not open file %s\r\n", filename);
    }
}


int main(void)
{
    SYSTEM_Initialize();
    
    //Init SPI
    SPI1_initPins();
    SPI1_initHost();

    //Interrupt for card insert/remove
    CLC2_CLCI_SetInterruptHandler(&onCardChange);
    
    //Initialize Memory Card
    memCard_initDriver();
    
    // Enable the Global High Interrupts 
    INTERRUPT_GlobalInterruptHighEnable(); 

    // Enable the Global Low Interrupts 
    INTERRUPT_GlobalInterruptLowEnable(); 

    //Isolated unit tests for sub-systems
#ifdef UNIT_TEST_ENABLE
    unitTest_runSequence();
#endif

    bool hasPrinted = false;
    
    FATFS fs;
    
    FRESULT mntResult;
    
    const char* testFile = "test.txt";
    
    while(1)
    {
        if (memCard_getCardStatus() == STATUS_CARD_NOT_INIT)
        {
            //Card is plugged in
            disk_initialize();
        }
        else if (memCard_getCardStatus() == STATUS_CARD_READY)
        {
            if (!hasPrinted)
            {
                hasPrinted = true;
                
                mntResult = pf_mount(&fs);
#ifdef MEM_CARD_DEBUG_ENABLE
                printf("[DEBUG] pf_mount = %d\r\n", mntResult);
#endif
                
                //Mount the drive
                if (mntResult != 0x00)
                {
                    printf("[ERROR] Unable to Mount Drive!\r\n");
                }
                else
                {
                    modifyFile(testFile);
                }
            }
            
        }
        else if (memCard_getCardStatus() == STATUS_CARD_NONE)
        {
            hasPrinted = false;
        }
    }    
}