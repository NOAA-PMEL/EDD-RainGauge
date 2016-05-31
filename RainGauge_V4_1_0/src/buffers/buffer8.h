/** @file fuses.h
 *  @brief
 *
 *  @author Matt Casari, matthew.casari@noaa.org
 *  @date March 23, 2016
 *  @version 0.1.0
 *
 *  @copyright National Oceanic and Atmospheric Administration
 *  @copyright Pacific Marine Environmental Lab
 *  @copyright Environmental Development Division
 *
 *	@note
 *
 *  @bug  No known bugs
 */



#ifndef BUFFER8_H_
#define BUFFER8_H_
/************************************************************************
*							HEADER FILES
************************************************************************/
#ifndef PMEL
#include "../../test/msp430fr5969.h"
#else
#include <msp430fr5949.h>
#endif
/************************************************************************
*						STANDARD LIBRARIES
************************************************************************/
#include <stdint.h>

/************************************************************************
*							MACROS
************************************************************************/

#define BUFFER8_SIZE				(60)
#define ACTUAL_BUFFER8_SIZE		(BUFFER8_SIZE + 1)

#define BUFFER8_ERROR_EMPTY			(1)
#define BUFFER8_ERROR_FULL			(2)
#define BUFFER8_NOT_FULL				(0)
#define BUFFER8_NOT_EMPTY			(BUFFER8_NOT_FULL)
#define BUFFER8_IS_EMPTY				(1)
#define BUFFER8_IS_FULL				(2)

/** @brief uint8_t circular buffer
 * 
 * This is the circular buffer type that stores uint8_t values
 *
 */
typedef struct _CircularBuffer8_s
{
	char buffer[BUFFER8_SIZE];
	uint16_t size;
	uint16_t read;
	uint16_t write;

}CircularBuffer8_s;


/************************************************************************
*					GLOBAL FUNCTION PROTOTYPES
************************************************************************/
/** @brief Add uint8_t to buffer
 *
 *	Adds a single uint8_t value to the buffer
 *
 *  @param *buf pointer to the uint8_t buffer
 *  @param value uint8_t value to add
 *
 *  @return status
 */
int8_t Buffer8_Put(CircularBuffer8_s *buf, uint8_t value);
/** @brief Add uint8_t to buffer and wrap
 *
 *	Adds a single uint8_t value to the buffer and 
 *  overwrite if necessary.
 *
 *  @param *buf pointer to the uint8_t buffer
 *  @param value uint8_t value to add
 *
 *  @return status
 */
int8_t Buffer8_Put_Circular(CircularBuffer8_s *buf, uint8_t value);
/** @brief Get uint8_t from buffer
 *
 *	Retreives one uint8_t value from the buffer
 *
 *  @param *buf pointer to the uint8_t buffer
 *  @param *value pointer to the uint8_t variable
 *
 *  @return status
 */
int8_t Buffer8_Get(CircularBuffer8_s *buf, uint8_t *value);
/** @brief Get uint8_t from requested buffer location
 *
 *	Retreives one uint8_t value from the buffer at the requested
 *  buffer location 
 *
 *  @param *buf pointer to the uint8_t buffer
 *  @param *value index location of value to retreive
 *
 *  @return status
 */
int8_t Buffer8_GetRequested(CircularBuffer8_s *buf, uint8_t requested, uint8_t *value);
/** @brief Clear the buffer
 *
 *	Clear the uint8_t buffer
 *
 *  @param *buf pointer to the uint8_t buffer
 *
 *  @return status
 */
int8_t Buffer8_Clear(CircularBuffer8_s *buf);
/** @brief Tests for full buffer
 *
 *	Checks the buffer to see if it is full
 *
 *  @param *buf pointer to the uint8_t buffer
 *
 *  @return status
 */
int8_t Buffer8_IsFull(CircularBuffer8_s *buf);
/** @brief Test for empty buffer
 *
 *	Checks the buffer to see if it is empty
 *
 *  @param *buf pointer to the uint8_t buffer
 *
 *  @return status
 */
int8_t Buffer8_IsEmpty(CircularBuffer8_s *buf);


#endif

