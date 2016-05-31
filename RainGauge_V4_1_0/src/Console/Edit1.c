/** @file console.c
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
  uint8_t Intro[] = "\r\n>";
  uint8_t Outro[] = "\r\nExiting Console\r\n\r\n";
  char InputStr[64] = {0};
  uint8_t ctr = 0;
  uint8_t startChar = 0;
  uint8_t spaceChar = 0;
  uint8_t endChar = 0;
  uint8_t StartOfStringCalled = false;
  uint8_t DisplayFlag = false;
  uint32_t NextSecondTimer = 0;

  char OutputStr[64] = {0};
  uint8_t OutputStr_u[64] = {0};
  InputValue_t ConsoleState;
  
  /* Set the console state */
  ConsoleState = ConsoleWait;
    
  /* Clear the buffer */
  BufferC_Clear(&ConsoleData);
  
  /* Write the intro */
  UART_Write(&Intro[0],LENGTH_OF(Intro),UART_A1);
  
  /* clear the counter */
  ConsoleTimeoutCounter = 0;
  
  /* Wait for timeout or data entered */
  while(ConsoleTimeoutCounter < CONSOLE_TIMEOUT)
  { 
    /* If ther is UART data, retreive it and parse it */
    if(BufferC_IsEmpty(&ConsoleData) == BUFFER_C_NOT_EMPTY)
    {
      RTC.TimeAtCommand = SecondCounter;
      ConsoleTimeoutCounter = 0;
      BufferC_Get(&ConsoleData,&InputStr[ctr]);
      
      if(StartOfStringCalled == false)
      {
        switch(InputStr[ctr])
        {
          case 'A':
          case 'a':
            if(ConsoleState == ConsoleWait)
            {
              startChar = ctr;
              ConsoleState = ConstantsInput;
            }
            StartOfStringCalled = true;
            break;
          case 'T':
          case 't':
            startChar = ctr;
            ConsoleState = DateTimeInput;
            StartOfStringCalled = true;
            break;
            
          case 'S':
          case 's':
            startChar = ctr;
            ConsoleState = SerialInput;
            StartOfStringCalled = true;
            break;
          case ' ':
            spaceChar = ctr;
            break;
          case '\r':
          case '\n':
            endChar = ctr;
            break;
          case '?':         /* Look at current values */
            DisplayFlag = true;
            ConsoleState = ConsoleWait;
            break;
          case 0x18:        /* Ctrl-X to exit the console */
            ConsoleTimeoutCounter = CONSOLE_TIMEOUT;
            break;
          default:
            break;
        }
        
      } else {
        switch (InputStr[ctr])
        {
          case ' ':
          spaceChar = ctr;
          break;
        case '\r':
        case '\n':
          endChar = ctr;
          break;
        case '?':         /* Look at current values */
          DisplayFlag = true;
          ConsoleState = ConsoleWait;
          break;
        case 0x18:        /* Ctrl-X to exit the console */
          ConsoleTimeoutCounter = CONSOLE_TIMEOUT;
          break;
        default:
          break;
          
        }
      }
    
      ctr++;
    }
    
    switch(ConsoleState)
    {
      case ConstantsInput:
      DisplayFlag = CONSOLE_ConstantInput(&startChar,&spaceChar,&endChar,&InputStr[0],LENGTH_OF(InputStr));
        break;
      case DateTimeInput:
      DisplayFlag = CONSOLE_TimeInput(&startChar,&spaceChar,&endChar,&InputStr[0],LENGTH_OF(InputStr));
        break;
      case SerialInput:
      DisplayFlag = CONSOLE_SerialInput(&startChar, &spaceChar,&endChar,&InputStr[0],LENGTH_OF(InputStr));
        break;
      case ConsoleWait:
      default:
      
        break;
      
    }
    
    /* If display is requested, show the coefficient values & current time */
    /* Then clear the buffers */
    if(DisplayFlag == true)
    {
      /* Wait for two-ish seconds to let RTC adjust */
      NextSecondTimer = SecondCounter + 2;
      while(SecondCounter < NextSecondTimer);
      
      /* Write the Serial # */
      sprintf(OutputStr,"\r\n\r\nSerial Number: %s\r\n",serialNumber);
      memcpy(OutputStr_u,OutputStr,LENGTH_OF(OutputStr));
      UART_Write(&OutputStr_u[0],LENGTH_OF(OutputStr), UART_A1);
      
       /* Write the Firmware Version # */
      sprintf(OutputStr,"Firmware: %s\r\n",version);
      memcpy(OutputStr_u,OutputStr,LENGTH_OF(OutputStr));
      UART_Write(&OutputStr_u[0],LENGTH_OF(OutputStr), UART_A1);
      
      /* Write the DateTime to UART */
      sprintf(OutputStr, "%04x,%02x,%02x,%02x:%02x:%02x\r\n",RTCYEAR,RTCMON,RTCDAY,RTCHOUR,RTCMIN,RTCSEC);
      for(uint8_t i=0;i<64;i++)
      {
        OutputStr_u[i] = (uint8_t) OutputStr[i];
      }
      UART_Write(&OutputStr_u[0],LENGTH_OF(OutputStr), UART_A1);
      
      /* Write the Slope & Intercept values to UART */
      sprintf(OutputStr,"A0= %6.4f, A1= %6.2f\r\n",slope,intercept);
      memcpy(OutputStr_u,OutputStr,LENGTH_OF(OutputStr));
      UART_Write(&OutputStr_u[0],LENGTH_OF(OutputStr), UART_A1);
      
      
      /* Write the > character to screen */
      UART_Write(&Intro[0],LENGTH_OF(Intro),UART_A1);
      
      /* Clear all flags & buffers */
      DisplayFlag = false;
      StartOfStringCalled = false;
      spaceChar = 0;
      startChar = 0;
      endChar = 0;
      BufferC_Clear(&ConsoleData);
      ConsoleState = ConsoleWait;
      for(uint8_t i=0;i<64;i++)
      {
       InputStr[i] = 0;
      }
    }
  }

  /* Calculate the dm min & max values */
  if(intercept != 0)
  {
    dmMax = sqrt(-1*(slope/intercept));
    dmMax += 1500;
    dmMin = sqrt(slope/(550 - intercept ));
    dmMin -= 1500;
  }
  else
  {
    dmMin = NAN;
    dmMax = NAN;
  }

  /* Write the outro & exit the console */
  UART_Write(&Outro[0],LENGTH_OF(Outro),UART_A1);
  return;
}

/************************************************************************
*					STATIC FUNCTIONS
************************************************************************/
/** @brief Console Coefficient Input State
 *
 *  Enter console read/parse routine
 *
 *  @param *startChar Location of start of buffered string
 *  @param *spaceChar Location of space in buffered string
 *  @param *endChar Location of end character in buffered string
 *  @param *InputStr Location of buffered string
 *  @param length Length of buffered string
 *
 *  @return DispFlag Display flag to indicate to display coefficient data
 */
static uint8_t CONSOLE_ConstantInput(uint8_t *startChar,uint8_t *spaceChar, uint8_t *endChar, char *InputStr, uint8_t length)
{
  
  uint8_t InvalidMsg[] = "\r\nInvalid Input\r\n";
  uint8_t ReferenceNumber = 0;
  uint8_t DispFlag = false;
  
  char tempStr[32] = {0};
  float temp_f = 0;
  /* If there is a string with a valid format, parse it */
  if((*spaceChar > *startChar ) && (*endChar > *spaceChar))
  {
    if((*spaceChar - *startChar) == 2)
    {
      ReferenceNumber = InputStr[*spaceChar-1];
    }
    
    if((*endChar - *spaceChar) >= 2)
    {
      /* Look for bad values in the string */
      for(uint8_t i=(*spaceChar+1);i<=(*endChar-1);i++)
      {
        switch(InputStr[i])
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case '.':
          case 'e':
          case 'E':
          case '-':
          case '+':
            DispFlag = true;
            break;
          default:
            /* If bad value, alert the user and return */
            UART_Write(&InvalidMsg[0],LENGTH_OF(InvalidMsg),UART_A1);
            DispFlag = true;
            return DispFlag;
            break;
        }
      }
    
      /* Convert good string to Float */
      memcpy(tempStr,&InputStr[*spaceChar+1],(*endChar-*spaceChar -1));
      temp_f= atof(tempStr);
    }
    
    /* Overwrite the selected coefficient */
    switch(ReferenceNumber)
    {
      case '0':
        slope = temp_f;
        DispFlag = true;
        break;
      case '1':
        intercept = temp_f;
        DispFlag = true;
        break;
      default:
        UART_Write(&InvalidMsg[0],LENGTH_OF(InvalidMsg),UART_A1);
        break;
    }
    
    /* Clear the buffered data */
    *startChar = 0; 
    *spaceChar = 0;
    *endChar = 0;
    BufferC_Clear(&ConsoleData);
   
    for(uint8_t i=0;i<64;i++)
    {
     InputStr[i] = 0;
    }
  } 


  /* Look for bad strings */
  if(endChar == (spaceChar + 1))
  {
    UART_Write(&InvalidMsg[0],LENGTH_OF(InvalidMsg),UART_A1);
  }

  if(spaceChar == (startChar +1))
  {
    UART_Write(&InvalidMsg[0],LENGTH_OF(InvalidMsg),UART_A1);
      
  }
  
  return DispFlag;
}

/** @brief Console Time Input
 *
 *  Enter Real-Time Clock (RTC) Time values into micro
 *
 *  Enter console read/parse routine
 *
 *  @param *startChar Location of start of buffered string
 *  @param *spaceChar Location of space in buffered string
 *  @param *endChar Location of end character in buffered string
 *  @param *InputStr Location of buffered string
 *  @param length Length of buffered string
 *
 *  @return DispFlag Display flag to indicate to display coefficient data
 */
static uint8_t CONSOLE_TimeInput(uint8_t *startChar,uint8_t *spaceChar,uint8_t *endChar,char *InputStr,uint8_t length)
{
  
  uint8_t InvalidMsg[] = "\r\nInvalid Input\r\n";
  uint8_t DispFlag = false;
  uint8_t startIdx = 0;
  uint8_t ValidMessageFlag = false;
  
  
  /* Look for a valid string & Parse the time string */
  if( (*spaceChar > *startChar) && ( (*endChar - *spaceChar) == 15) )
  {
    startIdx = *spaceChar + 1;
    ValidMessageFlag = RTC_Set(&InputStr[startIdx]);
  }
  else
  {
    return DispFlag;
  }
  
  /* If there is an invalid message, report it */
  if(ValidMessageFlag == false)
  {
    UART_Write(&InvalidMsg[0],LENGTH_OF(InvalidMsg),UART_A1);
    DispFlag = false;
  }
  else
  {
    DispFlag = true;
  }

  /* Clear all input string variables */
  *startChar = 0; 
  *spaceChar = 0;
  *endChar = 0;
  BufferC_Clear(&ConsoleData);
  for(uint8_t i=0;i<64;i++)
  {
  InputStr[i] = 0;
  }

  return DispFlag;
}


/** @brief Console Serial Number Input
 *
 *  Enter Serial Number of the sensor
 *
 *  @param *startChar Location of start of buffered string
 *  @param *spaceChar Location of space in buffered string
 *  @param *endChar Location of end character in buffered string
 *  @param *InputStr Location of buffered string
 *  @param length Length of buffered string
 *
 *  @return DispFlag Display flag to indicate to display coefficient data
 */
static uint8_t CONSOLE_SerialInput(uint8_t *startChar,uint8_t *spaceChar,uint8_t *endChar,char *InputStr,uint8_t length)
{
  uint8_t InvalidLength[] = "String Too Long\r\n"; 
  uint8_t DispFlag = false;
  uint8_t ValLength = 0; 
  
  if((*spaceChar > *startChar ) && (*endChar > *spaceChar))
  {
    ValLength = (*endChar) - (*spaceChar +1);
    if(ValLength > 16)
    {
      UART_Write(&InvalidLength[0],LENGTH_OF(InvalidLength),UART_A1);
      return DispFlag;
    }
    for(uint8_t i=0;i<ValLength;i++)
    {
      serialNumber[i] = InputStr[*spaceChar+1+i];
    }
//    memcpy(&serialNumber,InputStr[*spaceChar+1],ValLength);
    DispFlag = true;
  }
  
  return DispFlag;
}