/** @file timer.h
 *  @brief Drivers for MSP430FR5949 Timers
 *
 *  @author Matt Casari, matthew.casari@noaa.org
 *  @date March 23, 2016
 *  @version 0.1.2
 *
 *  @copyright National Oceanic and Atmospheric Administration
 *  @copyright Pacific Marine Environmental Lab
 *  @copyright Environmental Development Division
 *
 *
 *  @bug  No known bugs
 */

#ifndef TIMER_H
#define TIMER_H

/************************************************************************
*						STANDARD LIBRARIES
************************************************************************/
#include <stdint.h>
#include "../../inc/includes.h"

#ifdef PMEL
#include <msp430fr5949.h>
#else
#include "../../test/msp430fr5969.h"
#endif
/************************************************************************
*							HEADER FILES
************************************************************************/
#include "../../inc/includes.h"

/************************************************************************
*							MACROS
************************************************************************/
#ifndef true
#define true				(1)
#endif

#ifndef false
#define	false				(0)
#endif


#define	SYS_FREQ			(8000000u)
#define SMCLK				(SYS_FREQ / 8)

// Timer A Definitions
#define TIMERA_FREQUENCY		(1000)							// 10millisecond timer
#define TIMERA_TEMP_CALC1		(SMCLK/TIMERA_FREQUENCY)
#define TIMERA_DELAY_COUNT		(TIMERA_TEMP_CALC1 / 8)

// Timer B Definitions
#define TIMERB_FREQUENCY		(1000)							// 1millisecond timer
#define TIMERB_TEMP_CALC1		(SMCLK/TIMERB_FREQUENCY)
#define TIMERB_DELAY_COUNT		(TIMERB_TEMP_CALC1 / 8)

/************************************************************************
*						GLOBAL FUNCTION PROTOTYPES
************************************************************************/
/** @brief Initialize Timer A
 *
 *	Initializes Timer A for use (freq set above in TIMERA_FREQUENCY)
 *
 *  @param none
 *
 *  @return none
 */
void TIMER_A0_Init ( void );

/** @brief Initialize Timer B
 *
 *	Initializes Timer B for use (freq set above in TIMERB_FREQUENCY)
 *
 *  @param none
 *
 *  @return none
 */
void TIMER_A1_Init ( void );

/** @brief TimerB X milli-second timer
 *
 *	Starts a one-shot timer for length milli-seconds in timer B
 *
 *  @param length Number of milli-seconds to set timer B for
 *
 *  @return none
 *
 *  @todo This function is restricts processor until complete, should be rethought
 */
void TIMER_B_MilliSecond( uint16_t length );

#endif
