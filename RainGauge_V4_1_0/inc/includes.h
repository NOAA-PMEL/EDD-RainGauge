
/** @file includes.h
 *  @brief Drivers for MSP430FR5969 GPIO
 *
 *  @author Matt Casari, matthew.casari@noaa.org
 *  @date October 7, 2015
 *  @version 0.1.1
 *
 *  @copyright National Oceanic and Atmospheric Administration
 *  @copyright Pacific Marine Environmental Lab
 *  @copyright Environmental Development Division
 *
 *  @bug  No known bugs
 */

#ifndef INCLUDES_H
#define INCLUDES_H


/*****************************  DEBUG  *********************************/
/* #define DEBUG */

/************************************************************************
*						STANDARD LIBRARIES
************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#ifndef PMEL
//#include "../test/msp430fr5969.h"
#else
#include <msp430fr5949.h>		/* Device Specific Header File  */
#endif

/************************************************************************
*							HEADER FILES
************************************************************************/
#include "../src/GPIO/gpio.h"			/* GPIO Driver Header file  */
#include "../src/UART/uart.h"	  		/* UART Driver Header file  */
#include "../src/Timer/timer.h" 		/* Timer Driver Header file */
#include "../src/Console/console.h"     /* Console header file      */
#include "../src/buffers/buffer_c.h"    /* Char buffer header file  */
#include "../src/buffers/buffer_f.h"    /* Float buffer header file */
#include "../src/buffers/buffer8.h"     /* uint8_t buffer header file */
#include "../src/buffers/buffer16.h"    /* uint16_t buffer header file */
#include "../src/rtc/rtc.h"             /* Real-Time Clock header file */
#include "../src/Stats/statistics.h"    /* Statistical Calculation header file */

/************************************************************************
*							CONSTANTS
************************************************************************/
#ifndef true
#define true				(1)
#endif

#ifndef false
#define	false				(0)
#endif

/************************************************************************
*							MACROS
************************************************************************/
/* Generic functions */
#define LENGTH_OF(x)				(sizeof(x)/sizeof(x[0]))
#define BIT_TO_MASK(a) 			 ( 1u << (a) )

/* Set up the wait loop delay values */
#define DELAY					(1000)

/* Debug Pin  */
#ifdef DEBUG
#define DEBUG_PORT  (1)
#define DEBUG_PIN   (6)
#endif 

/* RTC Offset Value */
#define RTCCALSVAL      (0)
#define RTCOFFSET       (70)
/************************************************************************
*							STRUCTURES
************************************************************************/
typedef struct _CircularBufferC_s CircularBuffer_t ;

typedef struct _RTCStruct_s RTCStruct_t;

/** @brief Hour Data Buffer
 * 
 * This is the hour data buffer which stores all statistical data
 * for the sensor each minute for the previous hour
 *
 */
typedef struct _SampleData_s {
  CircularBufferF_s Mean;   /** Mean Volume for 60 minuetes */
  CircularBufferF_s STD;    /** Standard Deviation for 60 minutes */
  CircularBufferF_s Min;    /** Min Volume for 60 minutes */
  CircularBufferF_s Max;    /** Max Volume for 60 minutes */
  CircularBuffer16_s Year;  /** Sample Year for 60 minutes */
  CircularBuffer8_s Month;  /** Sample Month for 60 minutes */
  CircularBuffer8_s Day;    /** Sample Day for 60 minutes */
  CircularBuffer8_s Hour;   /** Sample Hour for 60 minutes */
  CircularBuffer8_s Minute; /** Sample Minute for 60 minutes */
}SampleData_t;

/** @brief Minute Data Buffer
 *
 * This is the minute data buffer which stores all the count data
 * and DateTime information for sixty seconds within the current minute
 * The buffer is 5 deep to store 5 minutes of data in case the system is busy
 * at the minute interval.
 *
 */
typedef struct _CurrentData_s {
  //uint32_t Counts[5][60];   /** Count data for sixty seconds, 5 deep */
  uint32_t Counts[300];     /** Count data for 5 minutes (300 seconds) */
  uint16_t Year[5];         /** Year value, 5 deep */
  uint8_t Mon[5];           /** Month value, 5 deep */
  uint8_t Day[5];           /** Day value, 5 deep */
  uint8_t Hour[5];          /** Hour value, 5 deep */
  uint8_t Min[5];           /** Minute value, 5 deep */
  uint16_t StartIdx[5];      /** Second Start Index for each minute */
  uint16_t EndIdx[5];        /** Second End Index for each minute */
  uint8_t min;              /** Minutes index */
  uint16_t sec;              /** Seconds index */
  uint8_t numSamples;       /** Number of samples since last conversion */
  uint8_t lastSampleRecorded;   /** Last sample to be converted */
}CurrentData_t;

/** @brief System State Enumeration
 *
 *  All possible states of the sensor
 *
 */
typedef enum SysState {
  Sample,               /** Sampling State - System in Low Power Mode */
  Console,              /** Console State - Awaiting User Input */
  MinuteTimerRoutine,   /** Minute State - Converting data and storing in buffers */
  Transmit,             /** Transmit State - See Substates for choices */
  Offset            /** TimeOffset State - Update the time by +/- offset seconds */
} SystemState_t;

/** @brief Transmit State Enumeration
 *
 *  All possible states for transmitting data to user
 *
 */
typedef enum TransSubState{
  Counts,               /** Counts SubState - Transmit Counts & Seconds to user */
  Volume,               /** Volume SubState - Transmit Volume(mL) to user */
  Report,               /** Report SubState - Transmit 60 minute statistical data to user */
  Iridium,              /** Iridium SubState - Transmit 60 min. data in Iridium format */
  CurrentTime           /** Current Time SubState - Transmit DateTime from RTC to user */    
} TransSubState_t;
/************************************************************************
*							GLOBAL VARIABLES
************************************************************************/
#ifdef PMEL

/* Variables */
extern uint8_t serialNumber[];
extern uint8_t version[];

/* Flags and counters */
extern volatile uint32_t SensorCounter;
extern uint32_t SecondCounter;
extern uint32_t SumOfCount;
extern uint32_t MinuteSensorCounter;
extern volatile uint8_t ConsoleTimeoutCounter;
extern volatile uint8_t ConsoleCounter;
extern float slope;
extern float intercept;
extern float dmMin;
extern float dmMax;
extern uint8_t ClearBufferFlag;

/* Structures */
extern SystemState_t SystemState;
extern TransSubState_t TxSubState;
extern CircularBuffer_t ConsoleData;
extern CircularBuffer_t UartData;
extern RTCStruct_t RTC; 
extern CurrentData_t MinuteData;


#endif

#endif
