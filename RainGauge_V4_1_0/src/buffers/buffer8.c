/** @file buffer8.c
 *  @brief
 *
 *  @author Matt Casari, matthew.casari@noaa.gov
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

/************************************************************************
*					STATIC FUNCTION PROTOTYPES
************************************************************************/
#include "buffer8.h"

/************************************************************************
*					GLOBAL FUNCTIONS
************************************************************************/
int8_t Buffer8_Put(CircularBuffer8_s *buf, uint8_t value)
{
	uint8_t NextWrite = (buf->write + 1) % (ACTUAL_BUFFER8_SIZE);

	if(NextWrite == buf->read)
	{
		return BUFFER8_ERROR_FULL;
	}

	buf->buffer[buf->write] = value;
	buf->write = NextWrite;

	return 0;
}

int8_t Buffer8_Put_Circular(CircularBuffer8_s *buf, uint8_t value)
{
	uint8_t NextWrite = (buf->write + 1) % (BUFFER8_SIZE);                                                                          
	uint8_t NextRead = (buf->read + 1) % (BUFFER8_SIZE);
    
    if(NextWrite == buf->read) {
      buf->read = NextRead;
    }
    
    buf->buffer[buf->write] = value;
    buf->write = NextWrite;
	
	return 0;
}
      
int8_t Buffer8_Get(CircularBuffer8_s *buf, uint8_t *value)
{
	if(buf->read == buf->write)
	{
		return BUFFER8_ERROR_EMPTY;
	}

	*value = buf->buffer[buf->read];
	buf->read = (buf->read + 1) % ACTUAL_BUFFER8_SIZE;

	return 0;
}

int8_t Buffer8_GetRequested(CircularBuffer8_s *buf, uint8_t requested, uint8_t *value)
{
  *value = buf->buffer[requested];
  return 0;
}

int8_t Buffer8_Clear(CircularBuffer8_s *buf)
{
	uint16_t i = 0;



	for(i=0;i<BUFFER8_SIZE;i++)
	{
		buf->buffer[i] = 0;
	}
    
    buf->read = 1;
	buf->write = 0;
    buf->size = 0;

	return 0;
}

int8_t Buffer8_IsFull(CircularBuffer8_s *buf)
{
	uint16_t NextWrite = (buf->write + 1) %(ACTUAL_BUFFER8_SIZE);

	if(NextWrite == buf->read)
	{
		return BUFFER8_IS_FULL;
	}
	return BUFFER8_NOT_FULL;
}

int8_t Buffer8_IsEmpty(CircularBuffer8_s *buf)
{
	if(buf->write == buf->read)
	{
		return BUFFER8_IS_EMPTY;
	}

	return BUFFER8_NOT_EMPTY;
}
