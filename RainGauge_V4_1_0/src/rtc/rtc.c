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
#include "rtc.h"

/************************************************************************
*					STATIC FUNCTION PROTOTYPES
************************************************************************/
static uint8_t RTC_ParseString(char *RTCString,uint16_t *Year,uint8_t *Mon,uint8_t *Day,uint8_t *Hour,uint8_t *Min,uint8_t *Sec);
static uint8_t RTC_ConvertTwoHexToDec(uint8_t val);
static uint16_t RTC_ConvertFourHexToDec(uint16_t val);
static uint8_t RTC_ConvertDecToTwoHex(uint8_t val);
static uint16_t RTC_ConvertDecToFourHex(uint16_t val);
static void RTC_UpdateCurrent(void);
uint8_t GetDaysInMonth(uint8_t mon, uint8_t year);
/************************************************************************
*					GLOBAL FUNCTIONS
************************************************************************/
void RTC_Init(void)
{
  /* Configure RTC C                    */
  /* RTCTEIVE - Event Interrupt Enable  */
  /* RTCRDYIE - Ready Interrupt Enable  */
  /* RTCBCD - Timer on BCD Mode         */
  /* RTCHOLD - RTC Hold                 */
  RTCCTL01 = RTCTEVIE | RTCRDYIE | RTCBCD | RTCHOLD;  


  RTCYEAR = 0x1900;
  RTCMON = 0x1;
  RTCDAY = 0x1;
  RTCDOW = 0x01;
  RTCHOUR = 0x0;
  RTCMIN = 0x0;
  RTCSEC = 0x0;
  
  RTCAMIN = 0x01;                         /* Set 1 Minute Alarm */
  RTCAMIN |= 0x80;                        /* Enable Alaarm      */

  /* Set Calibration Freq to 512 */
  RTCCTL3 =  0x01;

  if(RTCCALSVAL == 1) {
    RTCCTL2 = RTCCALS | RTCOFFSET;
  } else {
    RTCCTL2 = RTCOFFSET;
  }
  
  RTCCTL01 &= ~(RTCHOLD);                 /* Start RTC          */
}
  
  
  
  
uint8_t RTC_Set(char *RTCString)
{
  uint8_t InvalidString[] = "\r\nInvalid Date/Time String\r\n";
  uint16_t Year;
  uint8_t Mon;
  uint8_t Day;
  uint8_t Hour;
  uint8_t Min;
  uint8_t Sec;
  uint8_t RTCStatus = false;
  uint8_t diffSec = 0;
  uint8_t tempSec = 0;
  uint8_t tempOffset = 0;
  uint16_t HighCentury = 0;
  uint16_t LowCentury = 0;
  uint16_t Decade = 0;
  uint16_t LowYear = 0;
  uint16_t low;
  uint16_t high;
  
  /*  Parse the Input String */
  RTCStatus = RTC_ParseString(RTCString,&Year,&Mon,&Day,&Hour,&Min,&Sec);
  
  /* Return Failure if Bad String */
  if(RTCStatus == RTC_FAIL)
  {
    UART_Write(&InvalidString[0],LENGTH_OF(InvalidString),UART_A1);
    return false;
  }
  
  /* Wait for the top of the second to race through calculations to reset on next second */
  tempSec = SecondCounter;
  while(SecondCounter == tempSec);
  
  
  /* Calculate Number of Seconds Difference between when message came in and present */
  diffSec = SecondCounter - RTC.TimeAtCommand; /* +1 for reset at next second */
  
  Sec += diffSec;
  if(Sec > 59)
  {
    tempOffset = Sec / 60;
    Sec = Sec % 60;
  } else {
    tempOffset = 0;
  }
  
  Min = Min + tempOffset;
  if(Min > 59)
  {
    tempOffset = Min / 60;
    Min = Min % 60;
  } else {
    tempOffset = 0;
  }
  
  Hour = Hour + tempOffset;
  if(Hour > 23)
  {
    tempOffset = Hour / 24;
    Hour = Hour % 24;
  } else {
    tempOffset = 0;
  }
  
  Day = Day + tempOffset;
  switch(Mon)
  {
    case 4:
      case 6:
      case 9:
      case 11:
        if(Day > 30)
        {
          Day = 1;
          Mon++;
        }
        break;
      case 2:
        if( (Year % 4) == 0)
        {
          if(Day > 29)
          {
            Day = 1; 
            Mon++;
          }
          
        } else {
          if(Day > 28)
          {
            Day = 1;
            Mon++;
          }
        }
        break;
      case 1:
      case 3:
      case 5:
      case 7:
      case 10:
      case 12:
        if(Day > 31)
        {
          Day = 1;
          Mon++;
        }
        break;
      default:
        return RTC_FAIL;
        break;
      
    }
  
  if(Mon > 12)
  {
    Mon = 1;
    Year++;
  }
  
  /* Convert to correct format */
  low = Sec %  10;
  high = Sec / 10;
  high = high << 4;
  RTC.Sec = high + low;
  
  low = Min % 10;
  high = Min / 10;
  high = high << 4;
  RTC.Min = high + low;
  
  low = Hour % 10;
  high = Hour / 10;
  high = high << 4;
  RTC.Hour = high + low;
  
  low = Day % 10;
  high = Day / 10;
  high = high << 4;
  RTC.Day = high + low;
  
  low = Mon % 10;
  high = Mon / 10;
  high = high << 4;
  RTC.Mon = high + low;
 
  
  /* Convert Year to Hex    */
  LowYear = Year % 10;
  Decade = (Year - LowYear) / 10;
  LowCentury = Decade / 10;
  HighCentury = LowCentury / 10;
  Decade = Decade % 10;
  LowCentury = LowCentury % 10;
  HighCentury = HighCentury % 10;
  
  Decade = Decade << 4;
  LowCentury = LowCentury << 8;
  HighCentury = HighCentury << 12;
  
  RTC.Year = HighCentury + LowCentury + Decade + LowYear;

  /* Set the flag to update */
  RTC.UpdateFlag = true;
  
  RTCSEC = RTC.Sec;
  RTCYEAR = RTC.Year;
  RTCMON = RTC.Mon;
  RTCDAY = RTC.Day;
  RTCHOUR = RTC.Hour;
  RTCMIN = RTC.Min;
  
  RTC.UpdateFlag = false;
  
  return true;
}


uint8_t RTC_Offset(int32_t offset) {
  int32_t JulianSecond = 0;
  int32_t JulianMinute = 0;
  int32_t JulianHour = 0;
  int32_t JulianDay = 0;
  int32_t temp = 0;
  uint16_t Year = 0;
  uint8_t Mon = 0;
  uint8_t Day = 0;
  uint8_t Hour = 0;
  uint8_t Min = 0;
  uint8_t Sec = 0;
  
  RTC_UpdateCurrent();

  Year = RTC_ConvertFourHexToDec(RTC.Year);
  Mon = RTC_ConvertTwoHexToDec(RTC.Mon);
  Day = RTC_ConvertTwoHexToDec(RTC.Day);
  Hour = RTC_ConvertTwoHexToDec(RTC.Hour);
  Min = RTC_ConvertTwoHexToDec(RTC.Min);
  Sec = RTC_ConvertTwoHexToDec(RTC.Sec);
  
  if((offset > 31536000) || (offset < (-31536000))) {
      return RTC_BAD_OFFSET;
  }
  
  /* Calculate the Juilan Day */
  for(uint8_t i=0;i<Mon;i++)
  {
      JulianDay += GetDaysInMonth(i,Year); 
  }
  JulianDay += Day;
  
  JulianSecond = (int32_t) (SecondCounter - RTC.TimeAtCommand); /* +1 for reset at next second */
  /* Convert to get the Julian Second */
  temp = JulianDay * 86400;
  JulianSecond += temp;		/* 86400 seconds in a day */
  temp = ((int32_t)Hour * 3600);
  JulianSecond += temp; 			/* 3600 Seconds in an hour */
  temp = ((int32_t)Min * 60);
  JulianSecond += temp;					/* 60 seoncs in a minute */
  JulianSecond += (int32_t)Sec;
  
  /* Add in the offset */
  JulianSecond += (int32_t)offset;
  
  /* Did we lose a year? Needs to be at least one day in year */
  if(JulianSecond < 86400) {
      Year -= 1;
      
      /* Look for leap year then adjust seconds */
      if((Year % 4) == 0) {
          JulianSecond = 31622400 + JulianSecond;
      } else {
          JulianSecond = 31536000 + JulianSecond;
      }
  }
  
  /* Calculate the Julian Minute, Hour & Day with offset */
  JulianMinute = (JulianSecond / 60);
  JulianHour = (JulianSecond / 3600);
  JulianDay = (JulianSecond / 86400);
  
  
  Sec = (uint8_t) (JulianSecond % 60);
  Min = (uint8_t) (JulianMinute % 60);
  Hour = (uint8_t) (JulianHour % 24);
  
  Mon = 1;
  uint8_t idx = 0;
  uint8_t NumDaysInMonth = 0;
  
  while(JulianDay > 0) {
    idx++;
    NumDaysInMonth = GetDaysInMonth(idx,Year);
  
    if(JulianDay > NumDaysInMonth)
    {
        JulianDay -= NumDaysInMonth;
        
    } else {
        Day = (uint8_t) JulianDay;
        JulianDay = 0;
    }
  }
  
  Mon = idx;
  
  if(Mon > 12) {
    Mon = Mon % 12;
    Year += 1;
  }
  
  if(Mon < 1) {
    Mon = Mon % 12;
    Year -= 1;
  }
  
  
  RTC.Year = RTC_ConvertDecToFourHex(Year);
  RTC.Mon = RTC_ConvertDecToTwoHex(Mon);
  RTC.Day = RTC_ConvertDecToTwoHex(Day);
  RTC.Hour = RTC_ConvertDecToTwoHex(Hour);
  RTC.Min = RTC_ConvertDecToTwoHex(Min);
  RTC.Sec = RTC_ConvertDecToTwoHex(Sec);
 
  /* Set the flag to update */
  RTC.UpdateFlag = true;

  RTCSEC = RTC.Sec;
  RTCYEAR = RTC.Year;
  RTCMON = RTC.Mon;
  RTCDAY = RTC.Day;
  RTCHOUR = RTC.Hour;
  RTCMIN = RTC.Min;
  
  return 0;
}

uint8_t GetDaysInMonth(uint8_t mon, uint8_t year) {
	
	uint8_t days = 0;
	switch(mon)
	{
		case 4:
		case 6:
		case 9:
		case 11:
			days = 30;
			break;
		case 2:
			if((year % 4) == 0)
			{
				days = 29;
			} else {
				days = 28;
			}
			break;
		case 1:
		case 3:
		case 5:
		case 7:
        case 8:
		case 10:
		case 12:
			days = 31;
			break;
		default:
			break;
	}
	
	return days;
}
static uint8_t RTC_ParseString(char *RTCString,uint16_t *Year,uint8_t *Mon,uint8_t *Day,uint8_t *Hour,uint8_t *Min,uint8_t *Sec)
{
  char temp[4];
  uint16_t tYear = 0;
  uint8_t t0 = 0;
  
  
  for(uint8_t i=0;i<14;i++)
  {
    if( ( RTCString[i] < '0') || (RTCString[i] > '9') )
    {
      return RTC_FAIL;
    }
    
  }
  
  /* Parse Year & look for valid (Anything greater than last year, 2016+)   */
  memcpy(&temp,&RTCString[0],4);
  tYear = (uint16_t) atoi(temp);
  if(tYear > 2015)
  {
    *Year = tYear;
  } else {
    return RTC_FAIL;
  }
  memset(&temp,0,4);
  
  /* Parse Month & look for valid (1 - 12)  */
  memcpy(&temp,&RTCString[4],2);
  t0 = (uint8_t) atoi(temp);
  if(t0 > 12 || t0 < 1)
  {
    return RTC_FAIL;
  }
  *Mon = t0;
  memset(&temp,0,4);
    
  /* Parse Day & look for valid (based on month & year) */
  memcpy(&temp,&RTCString[6],2);
  t0 = (uint8_t) atoi(temp);
  switch(*Mon)
  {
    case 4:
    case 6:
    case 9:
    case 11:
      if(t0 > 30 || t0 < 1)
      {
        return RTC_FAIL;
      }
      break;
    case 2:
      if( (*Year % 4) == 0)
      {
        if(t0 > 29 || t0 < 1)
        {
          return RTC_FAIL;
        }
        
      } else {
        if(t0> 28 || t0 < 1)
        {
          return RTC_FAIL;
        }
      }
      break;
    case 1:
    case 3:
    case 5:
    case 7:
    case 10:
    case 12:
      if(t0 > 31 || t0 < 1)
      {
        return RTC_FAIL;
      }
      break;
    default:
      return RTC_FAIL;
      break;
    
  }
  *Day = t0;
  memset(&temp,0,4);

  /* Parse Hour & look for valid (0 - 23) */
  memcpy(&temp,&RTCString[8],2);
  t0 = (uint8_t) atoi(temp);
  if(t0 > 23)
  {
    return RTC_FAIL;
  }
  *Hour = t0;
  memset(&temp,0,4);

  /* Parse Min & look for valid (0 - 59) */
  memcpy(&temp,&RTCString[10],2);
  t0 = (uint8_t) atoi(temp);
  if(t0 > 59 )
  {
    return RTC_FAIL;
  }
  *Min = t0;
  memset(&temp,0,4);
  
  /* Parse Sec & look for valid (0 - 59) */
  memcpy(&temp,&RTCString[12],2);
  t0 = (uint8_t) atoi(temp);
  if(t0 > 59)
  {
    return RTC_FAIL;
  }
  *Sec = t0;
  memset(&temp,0,4);
   
   
  return RTC_OK;
}


static void RTC_UpdateCurrent(void) {
    RTC.Year = RTCYEAR;
    RTC.Mon = RTCMON;
    RTC.Day = RTCDAY;
    RTC.Hour = RTCHOUR;
    RTC.Min = RTCMIN;
    RTC.Sec = RTCSEC;
  
    return;
}

static uint8_t RTC_ConvertTwoHexToDec(uint8_t val){
  uint8_t low = 0;
  uint8_t high = 0;
  
  low = val & 0x0F;
  high = val & 0xF0;
  high = high >> 4;
  high *= 10;
  
  return (high + low);
}


static uint16_t RTC_ConvertFourHexToDec(uint16_t val) {
  uint16_t ones = 0;
  uint16_t tens = 0;
  uint16_t hundreds = 0;
  uint16_t thousands = 0;
  
  ones = val & 0x000F;
  
  tens = val & 0x00F0;
  tens = tens >> 4;
  tens *= 10;
  
  hundreds = val & 0x0F00;
  hundreds = hundreds >> 8;
  hundreds *= 100;
  
  thousands = val & 0xF000;
  thousands = thousands >> 12;
  thousands *= 1000;
  
  return (thousands + hundreds + tens + ones);
}


static uint8_t RTC_ConvertDecToTwoHex(uint8_t val)
{
  uint8_t low = 0;
  uint8_t high = 0;
  
  low = val %  10;
  high = val / 10;
  high = high << 4;
  
  return (high + low);
}


static uint16_t RTC_ConvertDecToFourHex(uint16_t val)
{
  uint16_t ones = 0;
  uint16_t tens = 0;
  uint16_t hundreds = 0;
  uint16_t thousands = 0;
  
  ones = val % 10;
  tens = (val - ones) / 10;
  hundreds = tens / 10;
  thousands = hundreds / 10;
  tens = tens % 10;
  hundreds = hundreds % 10;
  thousands = thousands % 10;
  
  tens = tens << 4;
  hundreds = hundreds << 8;
  thousands = thousands << 12;  
  
  return thousands + hundreds + tens + ones;
}




#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
  switch(__even_in_range(RTCIV, RTCIV_RT1PSIFG))
  {
    case RTCIV_NONE:
      break;
    case RTCIV_RTCOFIFG:    /* Oscillator Failure   */
      break;
    case RTCIV_RTCRDYIFG:   /* Second Timer         */
      if(SystemState != MinuteTimerRoutine) {
        /* Increment the second count & roll-over if needed */
        MinuteData.sec++;
        if(MinuteData.sec >= 300) {
          MinuteData.sec = 0;
        }
        
        /* Calculate the instantaneous Data */
        if( (0xFFFFFFFF - SumOfCount) > SensorCounter)
        {
          SumOfCount += SensorCounter;
         
        }
        else
        {
          SumOfCount = 0xFFFFFFFF;
        }
        
        /* Update Counters    */
        SensorCounter = 0;
        
        SecondCounter++;
        ConsoleTimeoutCounter++;
      }
#ifdef DEBUG
      GPIO_TogglePin(DEBUG_PORT,DEBUG_PIN);
      
#endif
      break;
      
    /*  Minute Timer */
    case RTCIV_RTCTEVIFG:   /* RTC Interval Timer Flag  */
      /* Minute timer  */
    
      /* Update the Minute Data values */
      MinuteData.EndIdx[MinuteData.min]= MinuteData.sec++;
      if(MinuteData.sec >= 300) {
        MinuteData.sec = 0;
      }

      /* Compute Second Information */
      /* Minute Interrupt precedes Second Interrupt */
      if( (0xFFFFFFFF - SumOfCount) > SensorCounter)
      {
        SumOfCount += SensorCounter;
       
      }
      else
      {
        SumOfCount = 0xFFFFFFFF;
      }
      
      /* Update Counters    */
      SensorCounter = 0;
      
      SecondCounter++;
      ConsoleTimeoutCounter++;
      
      /* Set to Run Minute Routine */
      if(SystemState == Sample) {
        SystemState = MinuteTimerRoutine;
      }

      /* Grab the date/time */
      MinuteData.Year[MinuteData.min] = RTCYEAR;
      MinuteData.Mon[MinuteData.min] = RTCMON;
      MinuteData.Day[MinuteData.min] = RTCDAY;
      MinuteData.Hour[MinuteData.min] = RTCHOUR;
      MinuteData.Min[MinuteData.min] = RTCMIN;
      
      MinuteData.min++;
      if(MinuteData.min > 4)
      {
        MinuteData.min = 0;
      }
      MinuteData.StartIdx[MinuteData.min] = MinuteData.sec;
      /* Clear the Seconds counter and increment the minute in the temp data buffer */


      /* Increment the number of temp samples collected counter */
      MinuteData.numSamples++;
      
      /* Set the minute flag */
      MinuteFlag = true;
      
      /* Exit from Low Power Mode */
      __low_power_mode_off_on_exit();
      break;
    case RTCIV_RTCAIFG:     /* RTC User Alarm   */
      __no_operation();
      break;
    case RTCIV_RT0PSIFG:    /* RTC Prescaler 0  */
      break;
    case RTCIV_RT1PSIFG:    /* RTC Prescalser 1 */
      break;
    default:
      break;   
  }
}



