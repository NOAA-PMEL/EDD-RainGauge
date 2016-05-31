/** @file rtc.h
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
#ifndef rtc_H
#define rtc_H

/************************************************************************
*							HEADER FILES
************************************************************************/
#include "../../inc/includes.h"

#ifndef PMEL
#include "../test/msp430fr5969.h"
#else
#include <msp430fr5949.h>
#endif

/************************************************************************
*						STANDARD LIBRARIES
************************************************************************/
#include <stdint.h>


/************************************************************************
*						    MACROS 
************************************************************************/
#define RTC_OK      (0)
#define RTC_FAIL    (1)
#define RTC_BAD_OFFSET (2)

/** @brief DateTime Structure
 *
 * Structure for sensor Data & Time values
 *
 */
typedef struct _RTCStruct_s
{
  uint16_t Year;            /** 4 digit year to set */
  uint8_t Mon;              /** 2 digit Month to set */
  uint8_t Day;              /** 2 digit Day to set */
  uint8_t Hour;             /** 2 digit Hour to set (24 hour clock)*/
  uint8_t Min;              /** 2 digit Minute to set */
  uint8_t Sec;              /** 2 digit second to set */
  uint8_t UpdateFlag;       /** Flag to update RTC */
  uint8_t TimeAtCommand;    /** RTC Time at receiving update time, used for offset */
}RTCStruct_s;


/** @brief Initialize the RTC
 *
 *	Sets the RTC up based on user provided time
 *
 *
 *  @return none
 */
void RTC_Init(void);


/** @brief Set the RTC 
 *  
 *  Sets the RTC to the Date/Time in String
 *  String format should be YYYYMMDDHHMMSS
 *
 * @param DateTime String
 *
 * @return Status
 */
uint8_t RTC_Set(char *RTCString);


uint8_t RTC_Offset(int32_t offset);

#endif // rtc_H
