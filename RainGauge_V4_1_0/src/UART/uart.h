/** @file uart.h
 *  @brief
 *
 *  @author Matt Casari, matthew.casari@noaa.org
 *  @date March 23, 2016
 *  @version 0.1.1
 *
 *  @copyright National Oceanic and Atmospheric Administration
 *  @copyright Pacific Marine Environmental Lab
 *  @copyright Environmental Development Division
 *
 *	@note
 *
 *  @bug  No known bugs
 */



#ifndef UART_H_
#define UART_H_
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
*							MACROS
************************************************************************/
#define UART_OK		(0)
#define UART_FAIL	(1)

/* UART USCI Port Definitions */
#define UART_A0			(0)
#define UART_A1			(1)

/* UART Pin Definitions */
#define UART_A0_TX		(BIT0)
#define UART_A0_RX		(BIT1)
#define UART_A1_TX		(BIT5)
#define UART_A1_RX		(BIT6)

/* Clk Definitions */
#define UART_CLK_UCLK	(0)
#define UART_CLK_ACLK	(1)
#define UART_CLK_SMCLK	(2)

/* Baudrate Definitions */
#define UART_BAUD_1200		(1200)
#define UART_BAUD_2400		(2400)
#define UART_BAUD_4800		(4800)
#define UART_BAUD_9600		(9600)
#define UART_BAUD_14400		(14400)
#define UART_BAUD_19200		(19200)
#define UART_BAUD_28800		(28800)
#define UART_BAUD_38400		(38400)
#define UART_BAUD_57600		(57600)
#define UART_BAUD_115200	(115200)
#define UART_BAUD_230400	(230400)
#define UART_BAUD_460800	(460800)

#define BAUDRATE_OK			(0)
#define BAUDRATE_FAIL		(1)

/* Clock Definitions */
#define CLK_32768			(32768)
#define CLK_1000000			(1000000)
#define CLK_1048576			(1048576)
#define CLK_4000000			(4000000)
#define CLK_4194304			(4194304)
#define CLK_8000000			(8000000)

#define CLOCK_OK			(0)
#define CLOCK_FAIL			(1)

/************************************************************************
*					GLOBAL FUNCTION PROTOTYPES
************************************************************************/
/** @brief Initialize the UART
 *
 *	Sets the UART up based on port, baudrate,clock frequency and clock
 *	source provided.
 *
 *  @param UartPort Uart Port
 *  @param Baudrate Common UART baudrate
 *  @param ClkFreq Frequency of peripheral clock
 *  @param Clk Clock Source (AClock, SMClock, etc.)
 *
 *  @return Status
 */
uint8_t UART_Init(uint8_t UartPort,uint32_t Baudrate,uint32_t ClkFreq, uint8_t Clk);

/** @brief Reads from UART
 *
 *	Reads one character from UART port
 *
 *  @param *value Pointer to char variable
 *  @param Port UART Port reading from
 *
 *  @return Status
 */
uint8_t UART_Read(uint8_t *value, uint8_t Port);

/** @brief Write one char to port
 *
 *	Writes only one char to port
 *
 *  @param value char value to transmit
 *  @param Port UART Port writing to
 *
 *  @return none
 */
uint8_t UART_WriteChar(uint8_t value, uint8_t Port);

/** @brief Write string to UART
 *
 *	Write a char array to the selected UART port
 *
 *  @param *value Pointer to the character array
 *  @param length length of string
 *  @param Port UART port selected
 *
 *  @return none
 */
uint8_t UART_Write(uint8_t *value, uint8_t length, uint8_t Port);

/** @brief Write string to UART with terminating Null
 *
 *	Write a char array to the selected UART port
 *  and follow with a NULL value
 *
 *  @param *value Pointer to the character array
 *  @param length length of string
 *  @param Port UART port selected
 *
 *  @return none
 */
uint8_t UART_WriteIncludeNull(uint8_t *value, uint8_t length, uint8_t Port);

uint8_t UART_WriteACK(uint8_t Port);
uint8_t UART_WriteNACK(uint8_t Port);
#endif

