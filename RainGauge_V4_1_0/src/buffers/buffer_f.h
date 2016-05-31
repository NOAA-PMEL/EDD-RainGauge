/** @file buffer_f.h
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

#ifndef BUFFER_F_H_
#define BUFFER_F_H_
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
#include <math.h>
/************************************************************************
*							MACROS
************************************************************************/

#define BUFFER_F_SIZE				(60)
#define ACTUAL_BUFFER_F_SIZE		(BUFFER_F_SIZE + 1)

#define BUFFER_F_ERROR_EMPTY			(1)
#define BUFFER_F_ERROR_FULL			(2)
#define BUFFER_F_NOT_FULL				(0)
#define BUFFER_F_NOT_EMPTY			(BUFFER_F_NOT_FULL)
#define BUFFER_F_IS_EMPTY				(1)
#define BUFFER_F_IS_FULL				(2)

/** @brief float circular buffer
 * 
 * This is the circular buffer type that stores float values
 *
 */
typedef struct _CircularBufferF_s
{
	float buffer[BUFFER_F_SIZE];
	uint16_t size;
	uint16_t read;
	uint16_t write;

}CircularBufferF_s;


/************************************************************************
*					GLOBAL FUNCTION PROTOTYPES
************************************************************************/
/** @brief Add float to buffer
 *
 *	Adds a single float value to the buffer
 *
 *  @param *buf pointer to the float buffer
 *  @param value float value to add
 *
 *  @return status
 */
int8_t BufferF_Put(CircularBufferF_s *buf, float value);
/** @brief Add float to buffer and wrap
 *
 *	Adds a single float value to the buffer and 
 *  overwrite if necessary.
 *
 *  @param *buf pointer to the float buffer
 *  @param value float value to add
 *
 *  @return status
 */
int8_t BufferF_Put_Circular(CircularBufferF_s *buf, float value);
/** @brief Get float from buffer
 *
 *	Retreives one float value from the buffer
 *
 *  @param *buf pointer to the float buffer
 *  @param *value pointer to the float variable
 *
 *  @return status
 */
int8_t BufferF_Get(CircularBufferF_s *buf, float *value);
/** @brief Get float from requested buffer location
 *
 *	Retreives one float value from the buffer at the requested
 *  buffer location 
 *
 *  @param *buf pointer to the float buffer
 *  @param *value index location of value to retreive
 *
 *  @return status
 */
int8_t BufferF_GetRequested(CircularBufferF_s *buf, uint8_t requested, float *value);
/** @brief Clear the buffer
 *
 *	Clear the float buffer
 *
 *  @param *buf pointer to the float buffer
 *
 *  @return status
 */
int8_t BufferF_Clear(CircularBufferF_s *buf);
/** @brief Tests for full buffer
 *
 *	Checks the buffer to see if it is full
 *
 *  @param *buf pointer to the float buffer
 *
 *  @return status
 */
int8_t BufferF_IsFull(CircularBufferF_s *buf);
/** @brief Test for empty buffer
 *
 *	Checks the buffer to see if it is empty
 *
 *  @param *buf pointer to the float buffer
 *
 *  @return status
 */
int8_t BufferF_IsEmpty(CircularBufferF_s *buf);
                                                     
#endif

