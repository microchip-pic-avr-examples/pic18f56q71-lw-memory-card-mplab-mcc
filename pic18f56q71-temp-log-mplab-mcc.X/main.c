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

#define READ_FILENAME "test_r.txt"
#define WRITE_FILENAME "test_w.txt"

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

void readFile(void)
{
    FRESULT result;
    unsigned int rxLen = 0;
    char rxBuffer[127];
    const char* filename = READ_FILENAME;
    
    printf("Reading file \"%s\"\r\n", filename);
    
    //Open a file
    result = pf_open(filename);
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[DEBUG] pf_open = %d\r\n", result);
#endif
    if (result == FR_OK)
    {
        if (pf_read(&rxBuffer[0], 126, &rxLen) == FR_OK)
        {
            rxBuffer[rxLen] = '\0';
            printf("Printing file \"%s\"\r\n> %s\r\n", filename, rxBuffer);
        }
        else
        {
            printf("[ERROR] Failed to open file\r\n");
        }
    }
}

void writeFile(void)
{
    FRESULT result;
    const char* filename = WRITE_FILENAME;
    const char* testWrite = "New data";
    const unsigned int wLen = 8;
    
    unsigned int bwLen = 0;

    printf("Overwriting file \"%s\"\r\n", filename);
    result = pf_open(filename);
#ifdef MEM_CARD_DEBUG_ENABLE
    printf("[DEBUG] pf_open = %d\r\n", result);
#endif
    if (result == FR_OK)
    {
        result = pf_lseek(0);
        if (result == FR_OK)
        {
            result = pf_write(&testWrite[0], wLen, &bwLen);
            if (result == FR_OK)
            {
                result = pf_write(0,0, &bwLen);
                if (result == FR_OK)
                {
                    printf("Write success!\r\n");
                }
                else
                {
                    printf("[ERROR] Failed to finalize write\r\n");
                }
            }
            else
            {
                printf("[ERROR] Failed to load write data\r\n");
            }
        }
        else
        {
            printf("[ERROR] Failed to seek file\r\n");
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
                    readFile();
                    writeFile();
                    readFile();
                }
            }
            
        }
        else if (memCard_getCardStatus() == STATUS_CARD_NONE)
        {
            hasPrinted = false;
        }
    }    
}