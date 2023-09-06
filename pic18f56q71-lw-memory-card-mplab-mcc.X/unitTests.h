#ifndef UNITTESTS_H
#define	UNITTESTS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdbool.h>
    
    //Runs all unit tests and prints results to the UART
    void unitTest_runSequence(void);
    
    //Tests the CRC7 Math
    bool unitTest_CRC7_test(void);

#ifdef	__cplusplus
}
#endif

#endif	/* UNITTESTS_H */

