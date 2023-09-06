#include "unitTests.h"
#include "memoryCard.h"
#include "mcc_generated_files/system/system.h"

#include <stdint.h>

//Runs all unit tests and prints results to the UART
void unitTest_runSequence(void)
{
    printf("Beginning Unit Tests...\r\n");
    
    printf("CRC7...\r\n");
    if (unitTest_CRC7_test())
    {
        printf("-- CRC7 Tests Passed --\r\n");
    }
}

//Tests the CRC7 Math
bool unitTest_CRC7_test(void)
{
    //Test Patterns
    uint8_t testPattern1[] = {0x40, 0x00, 0x00, 0x00, 0x00};
    uint8_t testPattern2[] = {0x51, 0x00, 0x00, 0x00, 0x00};
    uint8_t testPattern3[] = {0x11, 0x00, 0x00, 0x09, 0x00};
    
    //Test Pattern 1
    uint8_t result = memCard_runCRC7(&testPattern1[0], 5);
    if (result != 0x95)
    {
        printf("> Pattern 1 Mismatch: 0x%x\r\n", result);
        return false;
    }
    else
    {
        printf("Pattern 1 OK\r\n");
    }
    
    //Test Pattern 2
    result = memCard_runCRC7(&testPattern2[0], 5);
    if (result != 0x55)
    {
        printf("> Pattern 2 Mismatch: 0x%x\r\n", result);
        return false;
    }
    else
    {
        printf("Pattern 2 OK\r\n");
    }

    
    //Test Pattern 3
    result = memCard_runCRC7(&testPattern3[0], 5);
    if (result != 0x67)
    {
        printf("> Pattern 3 Mismatch: 0x%x\r\n", result);
        return false;
    }
    else
    {
        printf("Pattern 3 OK\r\n");
    }

    
    //All tests pass
    return true;
}
