/** @file buffer16.h
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



#ifndef BUFFER16_H_
#define BUFFER16_H_
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

#define BUFFER16_SIZE				(60)
#define ACTUAL_BUFFER16_SIZE		(BUFFER16_SIZE + 1)

#define BUFFER16_ERROR_EMPTY			(1)
#define BUFFER16_ERROR_FULL			(2)
#define BUFFER16_NOT_FULL				(0)
#define BUFFER16_NOT_EMPTY			(BUFFER16_NOT_FULL)
#define BUFFER16_IS_EMPTY				(1)
#define BUFFER16_IS_FULL				(2)

/** @brief uint16_t circular buffer
 * 
 * This is the circular buffer type that stores uint16_t values
 *
 */
typedef struct _CircularBuffer16_s
{
	uint16_t buffer[BUFFER16_SIZE];
	uint16_t size;
	uint16_t read;
	uint16_t write;

}CircularBuffer16_s;

/************************************************************************
*					GLOBAL FUNCTION PROTOTYPES
************************************************************************/
/** @brief Add uint16_t to buffer
 *
 *	Adds a single uint16_t value to the buffer
 *
 *  @param *buf pointer to the uint16_t buffer
 *  @param value uint16_t value to add
 *
 *  @return status
 */
int8_t Buffer16_Put(CircularBuffer16_s *buf, uint16_t value);
/** @brief Add uint16_t to buffer and wrap
 *
 *	Adds a single uint16_t value to the buffer and 
 *  overwrite if necessary.
 *
 *  @param *buf pointer to the uint16_t buffer
 *  @param value uint16_t value to add
 *
 *  @return status
 */
int8_t Buffer16_Put_Circular(CircularBuffer16_s *buf, uint16_t value);
/** @brief Get uint16_t from buffer
 *
 *	Retreives one uint16_t value from the buffer
 *
 *  @param *buf pointer to the uint16_t buffer
 *  @param *value pointer to the uint16_t variable
 *
 *  @return status
 */
int8_t Buffer16_Get(CircularBuffer16_s *buf, uint16_t *value);
/** @brief Get uint16_t from requested buffer location
 *
 *	Retreives one uint16_t value from the buffer at the requested
 *  buffer location 
 *
 *  @param *buf pointer to the uint16_t buffer
 *  @param *value index location of value to retreive
 *
 *  @return status
 */
int8_t Buffer16_GetRequested(CircularBuffer16_s *buf, uint8_t requested, uint16_t *value);
/** @brief Clear the buffer
 *
 *	Clear the uint16_t buffer
 *
 *  @param *buf pointer to the uint16_t buffer
 *
 *  @return status
 */
int8_t Buffer16_Clear(CircularBuffer16_s *buf);
/** @brief Tests for full buffer
 *
 *	Checks the buffer to see if it is full
 *
 *  @param *buf pointer to the uint16_t buffer
 *
 *  @return status
 */
int8_t Buffer16_IsFull(CircularBuffer16_s *buf);
/** @brief Test for empty buffer
 *
 *	Checks the buffer to see if it is empty
 *
 *  @param *buf pointer to the uint16_t buffer
 *
 *  @return status
 */
int8_t Buffer16_IsEmpty(CircularBuffer16_s *buf);


#endif

