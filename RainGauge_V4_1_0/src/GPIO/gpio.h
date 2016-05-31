/** @file GPIO.h
 *  @brief
 *
 *  @author Matt Casari, matthew.casari@noaa.org
 *  @date Dec 4, 2015
 *  @version
 *
 *  @copyright National Oceanic and Atmospheric Administration
 *  @copyright Pacific Marine Environmental Lab
 *  @copyright Environmental Development Division
 *
 *	@note
 *
 *  @bug  No known bugs
 */



#ifndef _GPIO_H
#define _GPIO_H
/************************************************************************
*							HEADER FILES
************************************************************************/

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
*							MACROS
************************************************************************/
#define GPIO_PORTP1		(1)
#define GPIO_PORTP2		(2)
#define GPIO_PORTP3		(3)
#define GPIO_PORTP4		(4)

#define GPIO_OK			(0)
#define GPIO_FAIL		(1)
#define GPIO_PORT_FAIL	(-1)


// Interrupt Edge Case
#define GPIO_EDGE_LOW_TO_HIGH	(0)
#define GPIO_EDGE_HIGH_TO_LOW	(1)

/************************************************************************
*					GLOBAL FUNCTION PROTOTYPES
************************************************************************/
/** @brief Initialize the GPIO
 *
 *	Clears all GPIO and sets to outputs
 *
 *  @param none
 *
 *  @return OKs
 */
uint8_t GPIO_Init(void);

/** @brief Set Output Pin
 *
 *	Sets the selected port/pin to output
 *
 *  @param Port GPIO port pin is located on
 *  @param Pin GPIO pin to set as output
 *
 *  @return status
 */
uint8_t GPIO_SetPinAsOutput(uint8_t Port, uint8_t Pin);

/** @brief Set Input Pin
 *
 *	Sets the selected port/pin as input
 *
 *  @param Port GPIO Port pin is located on
 *  @param Pin GPIO Pin to set as input
 *
 *  @return status
 */
uint8_t GPIO_SetPinAsInput( uint8_t Port, uint16_t Pin );

/** @brief Set Pin Output HIGH
 *
 *	Sets the selected pin output to HIGH
 *
 *  @param Port GPIO Port pin is located on
 *  @param Pin GPIO Pin to set high
 *
 *  @return status
 */
uint8_t GPIO_SetPin(uint8_t Port, uint8_t Pin);

/** @brief Set Pin Output LOW
 *
 *	Sets the selected pin output to LOW
 *
 *  @param Port GPIO Port pin is located on
 *  @param Pin GPIO Pin to set low
 *
 *  @return status
 */
uint8_t GPIO_ClearPin(uint8_t Port, uint8_t Pin);

/** @brief Set all Port Pins LOW
 *
 *	Sets all pins on selected port low
 *
 *  @param Port GPIO Port to set low
 *
 *  @return status
 */
uint8_t GPIO_SetAllPortOutputPinsLow(uint8_t Port);

/** @brief Set all Port Pins HIGH
 *
 *	Sets all pins on selected port high
 *
 *  @param Port GPIO Port to set high
 *
 *  @return status
 */
uint8_t GPIO_SetAllPortOutputPinsHigh(uint8_t Port);

/** @brief Read Input Pin value
 *
 *  Reads the value of the selected input pin
 *
 *  @param Port GPIO Port pin is located on
 *  @param Pin GPIO Pin to read
 *
 *  @return status
 */
int8_t GPIO_ReadInputPin( uint8_t Port, uint8_t Pin);

/** @brief Read all pins on port
 *
 *  Reads all the pins on the selected port
 *
 *  @param Port GPIO Port to Read
 *
 *  @return value
 */
int32_t GPIO_ReadInputPort(uint8_t Port);

/** @brief Toggle Output Pin
 *
 *	Toggle selected output pin
 *
 *  @param Port GPIO Port pin is located on
 *  @param Pin GPIO Pin to toggle
 *
 *  @return status
 */
uint8_t GPIO_TogglePin(uint8_t Port, uint8_t Pin);

/** @brief Create interrupt on Input Pin
 *
 *  Attaches an Interrupt on selected pin
 *
 *  @param Port GPIO Port pin is located on
 *  @param Pin GPIO Pin that interrupt is on
 *  @param edge Edge trigger interrupt is on (low -> high or high -> low)
 *
 *  @return status
 */
uint8_t GPIO_AttachInputInterrupt(uint8_t Port, uint16_t Pin, uint8_t edge);

/** @brief Removes interrupt on input pin
 *
 *  Detaches Interrupt previous set on pin
 *
 *  @param Port GPIO Port input pin is located on
 *  @param Pin GPIO Pin interrupt is located on
 *
 *  @return status
 */
uint8_t GPIO_DetachInputInterrupt(uint8_t Port, uint16_t Pin);

#endif
