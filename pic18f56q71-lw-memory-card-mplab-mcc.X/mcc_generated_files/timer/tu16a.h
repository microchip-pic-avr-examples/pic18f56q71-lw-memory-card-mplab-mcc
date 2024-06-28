/**
 * TU16A Generated Driver API Header File
 *
 * @file tu16a.h
 *  
 * @defgroup tu16a TU16A
 *
 * @brief This file contains the API prototypes for the TU16A module.
 *
 * @version TU16A Driver Version 2.1.0
 */
 

/*
© [2024] Microchip Technology Inc. and its subsidiaries.

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

#ifndef TU16A_H
#define TU16A_H

/**
  Section: Included Files
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "timer_interface.h"

/**
  Section: TU16A APIs
*/

/**
 *@ingroup tu16a
 *@struct TMR_INTERFACE
 *@brief Instance of the TMR_INTERFACE for the Universal Timer (UTMR) module.
 */
extern const struct TMR_INTERFACE TU16A;

/**
 *@ingroup tu16a
 *@brief Initializes the module register values.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_Initialize(void);

/**
 *@ingroup tu16a
 *@brief Starts the TU16A.
 *@pre Intialize the TU16A before calling this function.
 *@param None.
 *@return None.
 */
void TU16A_Start(void);

/**
 *@ingroup tu16a
 *@brief Stops the TU16A.
 *@pre Initialize the TU16A before calling this function.
 *@param None.
 *@return None.
 */
void TU16A_Stop(void);

/**
 *@ingroup tu16a
 *@brief Reads the uint32-bit value of the Capture register.
 *@pre Initialize the TU16A before calling this function.
 *@param None.
 *@return uint32-bit value of the Capture register.
 */
uint32_t TU16A_CaptureValueRead(void);

/**
 *@ingroup tu16a
 *@brief Captures the current timer value and stores it in the Capture register.
 *@pre Initialize the TU16A before calling this function.
 *@param None.
 *@return uint32-bit value of the Capture register.
 *@note When the UTMR Enable and Clock Synchronization (CSYNC) bits are set, it takes three timer clock cycles to synchronize the clocks.
 *      Clearing the UTMR Enable bit would require the selected clock source, especially the External Cock Sources (ERS), to supply at least three additional
 *      clock cycles to resolve the internal state. The user must be careful because if the timer is already running, any Stop/Reset-related ERS events that get
 *      processed will continue to affect the Run state of the timer.
 */
uint32_t TU16A_OnCommandCapture(void);

/**
 *@ingroup tu16a
 *@brief Reads the Timer Counter register value.
 *@pre Initialize the TU16A before calling this function.
 *@param None.
 *@return Current value of the Timer Counter register.
 *@note The Timer Counter register is not double-buffered, thus the timer must be
 *      stopped before writing to it. If the desired action is to clear the counter
 *      while the timer is running, consider using the CounterClear() API. If the
 *      desired action is to change the overall period of the timer (running or not),
 *      consider changing the Timer Period register (which is double-buffered) through the
 *      PeriodValueSet() API.
 */
uint32_t TU16A_Read(void);

/**
 *@ingroup tu16a
 *@brief Writes the Timer Counter register value.
 *@pre Initialize the TU16A before calling this function.
 *@param timerVal - Value to write to the Timer Counter register.
 *@return None.
 *@note The Timer Counter register is not double-buffered, thus the timer must be
 *      stopped before writing to it. If the desired action is to clear the counter
 *      while the timer is running, consider using the CounterClear() API. If the
 *      desired action is to change the overall period of the timer (running or not),
 *      consider changing the Timer Period register (which is double-buffered) through the
 *      PeriodValueSet() API.
 */
void TU16A_Write(size_t timerVal);

/**
 *@ingroup tu16a
 *@brief Clears the Timer Counter and the internal prescaler counter to zero.
 *@pre Initialize the TU16A before calling this function.
 *@param None.
 *@return None.
 *@note When the UTMR Enable and Clock Synchronization (CSYNC) bits are set, it takes three timer clock cycles to synchronize the clocks.
 *      Clearing the UTMR Enable bit would require the selected clock source, especially the External Cock Sources (ERS), to supply at least three additional
 *      clock cycles to resolve the internal state. The user must be careful because if the timer is already running, any Stop/Reset-related ERS events that get
 *      processed will continue to affect the Run state of the timer.
 */
void TU16A_CounterClear(void);

/**
 *@ingroup tu16a
 *@brief Writes the value to the Period registers.
 *@pre Initialize the TU16A before calling this function.
 *@param prVal - Value to load to the Period register.
 *@return None.
 *@note Writing to the Timer Period register is double-buffered,
 *      thus stopping the UTMR is not required. This must be followed
 *      by a Reset event for the new period value to become effective.
 */
void TU16A_PeriodValueSet(uint32_t prVal);

/**
 *@ingroup tu16a
 *@brief Enables the Period register match interrupt.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_PRMatchInterruptEnable(void);

/**
 *@ingroup tu16a
 *@brief Disables the Period register match interrupt.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_PRMatchInterruptDisable(void);

/**
 *@ingroup tu16a
 *@brief Enables the zero condition match interrupt.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_ZeroInterruptEnable(void);

/**
 *@ingroup tu16a
 *@brief Disables the zero condition match interrupt.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_ZeroInterruptDisable(void);

/**
 *@ingroup tu16a
 *@brief Enables the capture interrupt.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_CaptureInterruptEnable(void);

/**
 *@ingroup tu16a
 *@brief Disables the capture interrupt.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_CaptureInterruptDisable(void);

/**
 *@ingroup tu16a
 *@brief Returns the Period Match Interrupt flag status.
 *@pre None.
 *@param None.
 *@retval True - The counter has incremented from PR-1 to PR.
 *@retval False - The counter has not incremented from PR-1 to PR since this bit was last cleared.
 */
bool TU16A_HasPRMatchOccured(void);

/**
 *@ingroup tu16a
 *@brief Returns the Zero Interrupt flag status.
 *@pre None.
 *@param None.
 *@retval True - The counter has reset or rolled over to zero.
 *@retval False - The counter has not reset or rolled over since this bit was last cleared.
 */
bool TU16A_HasResetOccured(void);

/**
 *@ingroup tu16a
 *@brief Returns the Capture Interrupt flag status.
 *@pre None.
 *@param None.
 *@retval True - A capture event has occurred.
 *@retval False - A capture event has not occurred since this bit was last cleared.
 */
bool TU16A_HasCaptureOccured(void);

/**
 *@ingroup tu16a
 *@brief Returns the UTMR running flag status.
 *@pre None.
 *@param None.
 *@retval True - UTMR is running and not being held in Reset by the External Reset Source (ERS).
 *@retval False - UTMR is not running or is held in Reset by the ERS.
 */
bool TU16A_IsTimerRunning(void);

/**
 *@ingroup tu16a
 *@brief Enables the UTMR main interrupt.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_InterruptEnable(void);

/**
 *@ingroup tu16a
 *@brief Disables the UTMR main interrupt.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_InterruptDisable(void);

/**
 *@ingroup tu16a
 *@brief Returns the status of the main UTMR interrupt flag.
 *@pre None.
 *@param None.
 *@retval True - Interrupt is enabled.
 *@retval False - Interrupt is disabled.
 */
bool TU16A_IsInterruptEnabled(void);

/**
 *@ingroup tu16a
 *@brief Clears the status bit of the UTMR module interrupt flags.
 *@pre None.
 *@param None.
 *@return None.
 */
void TU16A_InterruptFlagsClear(void);


#endif //TU16A_H
