/** @file console.c
 *  @brief
 *
 *  @author Matt Casari, matthew.casari@noaa.org
 *  @date May 20, 2016
 *  @version 0.2.0
 *
 *  @copyright National Oceanic and Atmospheric Administration
 *  @copyright Pacific Marine Environmental Lab
 *  @copyright Environmental Development Division
 *
 *	@note
 *
 *  @bug  No known bugs
 */
#include "console.h"

/************************************************************************
*					STATIC FUNCTION PROTOTYPES
************************************************************************/
static uint8_t CONSOLE_ConstantInput(uint8_t *startChar,uint8_t *spaceChar, uint8_t *endChar, char *InputStr, uint8_t length);
static uint8_t CONSOLE_TimeInput(uint8_t *startChar,uint8_t *spaceChar,uint8_t *endChar,char *InputStr,uint8_t length);
static uint8_t CONSOLE_SerialInput(uint8_t *startChar,uint8_t *spaceChar,uint8_t *endChar,char *InputStr,uint8_t length);

/************************************************************************
*					GLOBAL FUNCTIONS
************************************************************************/
void CONSOLE_Main(void)
{
	char InputStr[64] = {0};
	uint8_t startChar = 0;
	uint8_t spaceChar = 0;
	uint8_t endChar = 0;
	uint8_t StartOfStringCalled = false;
	uint8_t DisplayFlag = false;
	