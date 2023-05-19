/**
 * Generated Pins header File
 * 
 * @file pins.h
 * 
 * @defgroup  pinsdriver Pins Driver
 * 
 * @brief This is generated driver header for pins. 
 *        This header file provides APIs for all pins selected in the GUI.
 *
 * @version Driver Version  3.1.0
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

#ifndef PINS_H
#define PINS_H

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set RA1 aliases
#define CARD_DETECT_TRIS                 TRISAbits.TRISA1
#define CARD_DETECT_LAT                  LATAbits.LATA1
#define CARD_DETECT_PORT                 PORTAbits.RA1
#define CARD_DETECT_WPU                  WPUAbits.WPUA1
#define CARD_DETECT_OD                   ODCONAbits.ODCA1
#define CARD_DETECT_ANS                  ANSELAbits.ANSELA1
#define CARD_DETECT_SetHigh()            do { LATAbits.LATA1 = 1; } while(0)
#define CARD_DETECT_SetLow()             do { LATAbits.LATA1 = 0; } while(0)
#define CARD_DETECT_Toggle()             do { LATAbits.LATA1 = ~LATAbits.LATA1; } while(0)
#define CARD_DETECT_GetValue()           PORTAbits.RA1
#define CARD_DETECT_SetDigitalInput()    do { TRISAbits.TRISA1 = 1; } while(0)
#define CARD_DETECT_SetDigitalOutput()   do { TRISAbits.TRISA1 = 0; } while(0)
#define CARD_DETECT_SetPullup()          do { WPUAbits.WPUA1 = 1; } while(0)
#define CARD_DETECT_ResetPullup()        do { WPUAbits.WPUA1 = 0; } while(0)
#define CARD_DETECT_SetPushPull()        do { ODCONAbits.ODCA1 = 0; } while(0)
#define CARD_DETECT_SetOpenDrain()       do { ODCONAbits.ODCA1 = 1; } while(0)
#define CARD_DETECT_SetAnalogMode()      do { ANSELAbits.ANSELA1 = 1; } while(0)
#define CARD_DETECT_SetDigitalMode()     do { ANSELAbits.ANSELA1 = 0; } while(0)
#define RA1_SetInterruptHandler  CARD_DETECT_SetInterruptHandler

// get/set RA5 aliases
#define CARD_CS_TRIS                 TRISAbits.TRISA5
#define CARD_CS_LAT                  LATAbits.LATA5
#define CARD_CS_PORT                 PORTAbits.RA5
#define CARD_CS_WPU                  WPUAbits.WPUA5
#define CARD_CS_OD                   ODCONAbits.ODCA5
#define CARD_CS_ANS                  ANSELAbits.ANSELA5
#define CARD_CS_SetHigh()            do { LATAbits.LATA5 = 1; } while(0)
#define CARD_CS_SetLow()             do { LATAbits.LATA5 = 0; } while(0)
#define CARD_CS_Toggle()             do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define CARD_CS_GetValue()           PORTAbits.RA5
#define CARD_CS_SetDigitalInput()    do { TRISAbits.TRISA5 = 1; } while(0)
#define CARD_CS_SetDigitalOutput()   do { TRISAbits.TRISA5 = 0; } while(0)
#define CARD_CS_SetPullup()          do { WPUAbits.WPUA5 = 1; } while(0)
#define CARD_CS_ResetPullup()        do { WPUAbits.WPUA5 = 0; } while(0)
#define CARD_CS_SetPushPull()        do { ODCONAbits.ODCA5 = 0; } while(0)
#define CARD_CS_SetOpenDrain()       do { ODCONAbits.ODCA5 = 1; } while(0)
#define CARD_CS_SetAnalogMode()      do { ANSELAbits.ANSELA5 = 1; } while(0)
#define CARD_CS_SetDigitalMode()     do { ANSELAbits.ANSELA5 = 0; } while(0)

// get/set RB4 aliases
#define UART_TX_TRIS                 TRISBbits.TRISB4
#define UART_TX_LAT                  LATBbits.LATB4
#define UART_TX_PORT                 PORTBbits.RB4
#define UART_TX_WPU                  WPUBbits.WPUB4
#define UART_TX_OD                   ODCONBbits.ODCB4
#define UART_TX_ANS                  ANSELBbits.ANSELB4
#define UART_TX_SetHigh()            do { LATBbits.LATB4 = 1; } while(0)
#define UART_TX_SetLow()             do { LATBbits.LATB4 = 0; } while(0)
#define UART_TX_Toggle()             do { LATBbits.LATB4 = ~LATBbits.LATB4; } while(0)
#define UART_TX_GetValue()           PORTBbits.RB4
#define UART_TX_SetDigitalInput()    do { TRISBbits.TRISB4 = 1; } while(0)
#define UART_TX_SetDigitalOutput()   do { TRISBbits.TRISB4 = 0; } while(0)
#define UART_TX_SetPullup()          do { WPUBbits.WPUB4 = 1; } while(0)
#define UART_TX_ResetPullup()        do { WPUBbits.WPUB4 = 0; } while(0)
#define UART_TX_SetPushPull()        do { ODCONBbits.ODCB4 = 0; } while(0)
#define UART_TX_SetOpenDrain()       do { ODCONBbits.ODCB4 = 1; } while(0)
#define UART_TX_SetAnalogMode()      do { ANSELBbits.ANSELB4 = 1; } while(0)
#define UART_TX_SetDigitalMode()     do { ANSELBbits.ANSELB4 = 0; } while(0)

// get/set RB5 aliases
#define UART_RX_TRIS                 TRISBbits.TRISB5
#define UART_RX_LAT                  LATBbits.LATB5
#define UART_RX_PORT                 PORTBbits.RB5
#define UART_RX_WPU                  WPUBbits.WPUB5
#define UART_RX_OD                   ODCONBbits.ODCB5
#define UART_RX_ANS                  ANSELBbits.ANSELB5
#define UART_RX_SetHigh()            do { LATBbits.LATB5 = 1; } while(0)
#define UART_RX_SetLow()             do { LATBbits.LATB5 = 0; } while(0)
#define UART_RX_Toggle()             do { LATBbits.LATB5 = ~LATBbits.LATB5; } while(0)
#define UART_RX_GetValue()           PORTBbits.RB5
#define UART_RX_SetDigitalInput()    do { TRISBbits.TRISB5 = 1; } while(0)
#define UART_RX_SetDigitalOutput()   do { TRISBbits.TRISB5 = 0; } while(0)
#define UART_RX_SetPullup()          do { WPUBbits.WPUB5 = 1; } while(0)
#define UART_RX_ResetPullup()        do { WPUBbits.WPUB5 = 0; } while(0)
#define UART_RX_SetPushPull()        do { ODCONBbits.ODCB5 = 0; } while(0)
#define UART_RX_SetOpenDrain()       do { ODCONBbits.ODCB5 = 1; } while(0)
#define UART_RX_SetAnalogMode()      do { ANSELBbits.ANSELB5 = 1; } while(0)
#define UART_RX_SetDigitalMode()     do { ANSELBbits.ANSELB5 = 0; } while(0)

/**
 * @ingroup  pinsdriver
 * @brief GPIO and peripheral I/O initialization
 * @param none
 * @return none
 */
void PIN_MANAGER_Initialize (void);

/**
 * @ingroup  pinsdriver
 * @brief Interrupt on Change Handling routine
 * @param none
 * @return none
 */
void PIN_MANAGER_IOC(void);

/**
 * @ingroup  pinsdriver
 * @brief Interrupt on Change Handler for the CARD_DETECT pin functionality
 * @param none
 * @return none
 */
void CARD_DETECT_ISR(void);

/**
 * @ingroup  pinsdriver
 * @brief Interrupt Handler Setter for CARD_DETECT pin interrupt-on-change functionality.
 *        Allows selecting an interrupt handler for CARD_DETECT at application runtime
 * @pre Pins intializer called
 * @param InterruptHandler function pointer.
 * @return none
 */
void CARD_DETECT_SetInterruptHandler(void (* InterruptHandler)(void));

/**
 * @ingroup  pinsdriver
 * @brief Dynamic Interrupt Handler for CARD_DETECT pin.
 *        This is a dynamic interrupt handler to be used together with the CARD_DETECT_SetInterruptHandler() method.
 *        This handler is called every time the CARD_DETECT ISR is executed and allows any function to be registered at runtime.
 * @pre Pins intializer called
 * @param none
 * @return none
 */
extern void (*CARD_DETECT_InterruptHandler)(void);

/**
 * @ingroup  pinsdriver
 * @brief Default Interrupt Handler for CARD_DETECT pin. 
 *        This is a predefined interrupt handler to be used together with the CARD_DETECT_SetInterruptHandler() method.
 *        This handler is called every time the CARD_DETECT ISR is executed. 
 * @pre Pins intializer called
 * @param none
 * @return none
 */
void CARD_DETECT_DefaultInterruptHandler(void);


#endif // PINS_H
/**
 End of File
*/