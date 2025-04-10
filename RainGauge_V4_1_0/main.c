/** @file main.c
 *
 *  @brief Main Function
 *
 *  @author Matt Casari, matthew.casari@noaa.org
 *  @date November 2, 2015
 *  @version 0.1.2
 *
 *  @copyright National Oceanic and Atmospheric Administration
 *  @copyright Pacific Marine Environmental Lab
 *  @copyright Engineering Development Division
 *
 *	@mainpage
 *
 *
 *  This Rain Gauge program takes the counter ouput from PMEL Rain Gauge v4.0
 *  capacitance sensor, counts the number of toggles per second and stores
 *  the counts and the number of elapsed seconds in seperate buffers until 
 *  the user commands a data retreival by passing a 'D' or 'd' character over
 *  the UART.  
 *  Return message is:
 *  @@@COUNT,TIME\r\n
 *  Where:
 *  @@@ is the message header
 *  COUNT is the 10 digit sensor count
 *  TIME is the 10 digit number of seconds integrated over
 *  
 *  RS-232 Info:
 *  Buad Rate - 9600
 *  Bits - 8
 *  Stop Bits - 1
 *  Parity - None 
 *
 *  @note To add FRAM capability, you must enable MPU in IAR (Project->Options->General Options->MPU/IPU->Support MPU)
 * 
 */


/*------------------- Connection Diagram------------------------------*/
/*
		       						 					
		                               
		    MSP430FR5949             
		 --------------------        
		 |                  |       
		 |                  |        
		 |              P3.0|---------------------- From Capacitance Sensor
		 |                  |
		 |                  |
		 |              P2.5|---------------------- UART Tx
 		 |              P2.6|---------------------- UART Rx
		 |                  | 		  
		 |              PJ.4|----------------------   LFXIn
		 |                  |						  32.768kHz Crystal
         |              PJ.5|----------------------   LFXOut
		 |                  |
		 --------------------
*/


#define VERSION     ("4.1.6")
/*****************************  Includes  *********************************/
#include "./inc/includes.h"

/************************ Function Prototypes *****************************/
void STATE_CheckRxBuffer(void);
void STATE_Sample(void);
void STATE_MinuteTimerRoutine(void);
void STATE_Transmit(uint32_t count, uint32_t seconds);
void STATE_TransmitVolume(float volume, uint32_t count, uint32_t seconds);
void STATE_TransmitReport(SampleData_t *Data);
void STATE_TransmitIridium(SampleData_t *Data);
void STATE_TransmitCurrentTime(void);
void SETUP_Clock(void);
void SETUP_GPIO(void);
void ClearBuffers(void);
float CalculateVolume(uint32_t count, uint32_t seconds);
void WD_Init(void);
void WD_Start(void);
void WD_Kick(void);
void populatehour(void);
void populateminute(void);
void RestartCounters(void);
#ifdef DEBUG
void BufferTest(void);
#endif

/***********************  Constants (In FRAM)  *****************************/
const uint8_t SensorPort = 3;
const uint8_t SensorPin = 0;

#ifdef DEBUG
const uint8_t DebugPort = 1;
const uint8_t DebugPin = 6;
#endif

/***********************  Persistents (In FRAM)  *****************************/
__persistent uint32_t SumOfCount = 0;
__persistent uint32_t MinuteSensorCounter = 0;
__persistent uint32_t SecondCounter = 0;
__persistent float slope = 1.412e12;
__persistent float intercept = -493.25;
__persistent float dmMin = 3.5540e4;    /* Value dm connot be less than */
__persistent float dmMax = 5.454e4;    /*  Value dm cannot exceed */
__persistent SampleData_t HourData;
__persistent RTCStruct_t RTC;
__persistent uint8_t version[] = VERSION;
__persistent uint8_t serialNumber[16] = "";
__persistent uint8_t ClearBufferFlag = false;

/*************************  Global variables  *****************************/
/* Counters */
volatile uint32_t SensorCounter;
volatile uint8_t ConsoleTimeoutCounter;
volatile uint8_t ConsoleCounter;

/* Structures */
CircularBufferC_s ConsoleData;
CircularBufferC_s UartData;
CurrentData_t MinuteData;

/* Enum types */
SystemState_t SystemState;
TransSubState_t TxSubState;

/* Flags */
uint8_t MinuteFlag;
uint8_t FreqPinActive;
char splash[] = "Startup";

/*******************************  MAIN  **********************************/
int main(void) {

  /* Configure MPU */
  __low_level_init();				// Setup FRAM
  WDTCTL = WDTPW | WDTHOLD;	        /* Pause the Watchdog */
  
  WD_Init();
  /* Debug Conditions */
#ifdef DEBUG
  BufferTest();
  MinuteData.lastSampleRecorded  = 0;
  MinuteData.numSamples = 0;
  MinuteData.min = 4;
  
  slope = 1.412e12;
  intercept = -493.25;
  dmMax = 5.304e4;
  dmMin = 3.704e4;
#endif 
  
 
  
  /* Configure GPIO */
  SETUP_GPIO();

  /* Setup the Clock */
  SETUP_Clock();
  
  /* Setup the RTC */
  RTC_Init();

  /* Clear the buffers */
  ClearBuffers();
    
  /* Configure the UART */
  UART_Init(UART_A1,UART_BAUD_9600,CLK_32768,UART_CLK_SMCLK);
 
  /* Enable the interrupts */
  __bis_SR_register(GIE);
  __no_operation();                        

  /* Set the startup State */
  SystemState = Sample;
  MinuteFlag = false;
  FreqPinActive = true;
  SensorCounter = 0;
  SecondCounter = 0;
  RTC.TimeAtCommand = SecondCounter;
  
  /* Start the Watchdog */
  WD_Start();

  /* Main loop */
  for(;;)
  {
    uint32_t temp_SumOfCount = 0;
    uint32_t temp_SecondsCounter = 0;

    float volume = 0.0;
    
    if(ClearBufferFlag == true)
    {
      ClearBuffers();
      ClearBufferFlag = false;
    }
    
    /* Reassert Input Pin Interrupt */
    MinuteData.Counts[MinuteData.sec] = 0;
    /* Kick the watchdog before entering LPM */
    WD_Kick();
    
    if(MinuteFlag == true) {
      SystemState = MinuteTimerRoutine;
    }
    /* Set LPM and wait for timer interrupt */
    if(SystemState == Sample) {
      BufferC_Clear(&UartData);
      __low_power_mode_3();
    }
    
    /* Kick the watchdog on coming out of LPM*/
    WD_Kick();
    
    /* Deassert Input Pin Interrupt */
    GPIO_DetachInputInterrupt(SensorPort,SensorPin);

    /* Clear Current Second Data so it doesn't report bad value */
    MinuteData.Counts[MinuteData.sec] = 0;
    
    /* Check the state and decode the buffer if needed */
    if((SystemState != Console) && (SystemState != MinuteTimerRoutine)) {
      do{
        WD_Kick();
        STATE_CheckRxBuffer();
      }while(BufferC_IsEmpty(&UartData) == BUFFER_C_NOT_EMPTY);
    }
    
    
    /* Check System State */
    switch(SystemState)
    {
      case Console:
        CONSOLE_Main();
        /* Kick the watchdog on exit of console */
        WD_Kick();
        
        BufferC_Clear(&UartData);
        SystemState = Sample;
        ConsoleCounter = 0;
        GPIO_AttachInputInterrupt(SensorPort, SensorPin,GPIO_EDGE_LOW_TO_HIGH);
        FreqPinActive = true;
        RestartCounters();
        break;
      case MinuteTimerRoutine:
        /* Grab the date/time */
        STATE_MinuteTimerRoutine();
        /* Set back to sampling state */
        SystemState = Sample;     
        /* Restart the GPIO Interrupt & clear counters */
        GPIO_AttachInputInterrupt(SensorPort, SensorPin,GPIO_EDGE_LOW_TO_HIGH);
        FreqPinActive = true;
        RestartCounters();
        MinuteFlag = false;
        break;
      case Transmit:
        if(SecondCounter < 1) {
          temp_SecondsCounter = SecondCounter + 1;
          while(SecondCounter < temp_SecondsCounter);
          FreqPinActive = true;
        } else {
          temp_SecondsCounter = SecondCounter;
          temp_SumOfCount = SumOfCount;
          FreqPinActive = true;
        }
                
        /* Kick the watchdog*/
        WD_Kick();
        switch(TxSubState)
        {
          case Counts:
            STATE_Transmit(temp_SumOfCount,temp_SecondsCounter);
            break;
          case Volume:
            volume = CalculateVolume(temp_SumOfCount, temp_SecondsCounter);
            STATE_TransmitVolume(volume,temp_SumOfCount,temp_SecondsCounter);
            break;
          case Report:
            STATE_TransmitReport(&HourData);
            if(SystemState != MinuteTimerRoutine) {
              SystemState = Sample;
            }
            break;
          case Iridium:
            STATE_TransmitIridium(&HourData);
            break;
          case CurrentTime:
            STATE_TransmitCurrentTime();
            break;
          default:
            SystemState = Sample;
            break;
        }
        /* Set back to sampling state */
        SystemState = Sample;
        /* Restart the GPIO Interrupt & clear counters */
        GPIO_AttachInputInterrupt(SensorPort, SensorPin,GPIO_EDGE_LOW_TO_HIGH);
        RestartCounters();
        break;
      case Offset:
        /* Set back to sampling state */
        SystemState = Sample;
        /* Restart the GPIO Interrupt & clear counters */
        GPIO_AttachInputInterrupt(SensorPort, SensorPin,GPIO_EDGE_LOW_TO_HIGH);
        RestartCounters();
        break;
      case Sample:
      default:
        break;
    }
    
  }
}

/** @brief Enters Minute Timer State    
 *
 *  Updates RTC values (if requested) and calculates stats
 *  on last 60 seconds of data.  Saves to associated buffer.
 *
 *  @param count Frequency count sum from sensor
 *  @param seconds Number of seconds of integration
 *
 *  @return Void
 */       
void STATE_MinuteTimerRoutine(void) {

  uint16_t Sec_StartIdx = 0;
  uint16_t Sec_EndIdx = 0;
  uint16_t Sec_Length = 0;
  uint8_t Min_Length = 0;
  uint8_t Min_Idx = 0;
  float vol[60] = {0};
  float Mean = 0;
  float STD = 0;
  float Min = 0;
  float Max = 0;
  
  /* Find the number of minute data buffers to be processed */
  Min_Length = MinuteData.numSamples;
  MinuteData.numSamples = 0;
 
  if(Min_Length <= MinuteData.min) {
    Min_Idx = MinuteData.min - Min_Length;
  } else {
    Min_Idx = 5 - (Min_Length - MinuteData.min);
  }
  
  /* Calculate the minute stats data for each minute buffer */
  for(uint8_t i=0;i<Min_Length;i++) {
    Sec_StartIdx = MinuteData.StartIdx[Min_Idx];
    Sec_EndIdx = MinuteData.EndIdx[Min_Idx];
    
    if(Sec_EndIdx >= Sec_StartIdx) {
      Sec_Length = Sec_EndIdx - Sec_StartIdx;
    } else {
      Sec_Length = 301 - (Sec_StartIdx - Sec_EndIdx);
    }
    
    for(uint8_t j=0;j<=Sec_Length;j++) {
      vol[j] = CalculateVolume(MinuteData.Counts[Sec_StartIdx],1);
      MinuteData.Counts[Sec_StartIdx] = 0;
      
      Sec_StartIdx++;
      Sec_StartIdx = Sec_StartIdx % 300;
    }
    
    
    /* Calculate Stats */
    STATS_CalculateMean(&vol[0], Sec_Length, &Mean);
    STATS_ComputeSTD(&vol[0],Sec_Length,Mean,&STD);
    STATS_FindMin(&vol[0],Sec_Length,&Min);
    STATS_FindMax(&vol[0],Sec_Length,&Max);

     /* Stuff the buffers */
    BufferF_Put_Circular(&HourData.Mean, Mean);
    BufferF_Put_Circular(&HourData.STD, STD);
    BufferF_Put_Circular(&HourData.Min, Min);
    BufferF_Put_Circular(&HourData.Max, Max);
    Buffer16_Put_Circular(&HourData.Year, MinuteData.Year[Min_Idx]);
    Buffer8_Put_Circular(&HourData.Month, MinuteData.Mon[Min_Idx]);
    Buffer8_Put_Circular(&HourData.Day, MinuteData.Day[Min_Idx]);
    Buffer8_Put_Circular(&HourData.Hour, MinuteData.Hour[Min_Idx]);
    Buffer8_Put_Circular(&HourData.Minute, MinuteData.Min[Min_Idx]);
  }
  
}

/** @brief Enters Transmit State
 *
 *  Preps the data for transmission and sends over the UART
 *
 *  @param count Frequency count sum from sensor
 *  @param seconds Number of seconds of integration
 *
 *  @return Void
 */
void STATE_Transmit(uint32_t count, uint32_t seconds){
  char sendString[64] = {0};
  uint8_t sendString_u[64] = {0};
  
  /* Setup the Load report string */
  sprintf(sendString,"@@@%10lu,",count);
  memcpy(sendString_u,sendString,64);
  UART_Write(&sendString_u[0],64,UART_A1);
  
  /* Write the second part of the load string */
  sprintf(sendString,"%10lu\r\n",seconds);
  memcpy(sendString_u,sendString,64);
  UART_Write(&sendString_u[0],64,UART_A1);
  
  return;
}

/** @brief Enters Transmit State
 *
 *  Preps the data for transmission and sends over the UART
 *
 *  @param volume Volume (in mL)
 *  @param seconds Numer of seconds of integration
 *
 *  @return Void
 */
void STATE_TransmitVolume(float volume, uint32_t count, uint32_t seconds){
  char sendString[64] = {0};
  uint8_t sendString_u[64] = {0};
  
  /*  Send the volume */
  sprintf(sendString,"@@@%7.2fmL,",volume);
  memcpy(sendString_u,sendString,64);
  UART_Write(&sendString_u[0],64,UART_A1);
  
  /*  Send the counts */
  sprintf(sendString,"%10lu,",count);
  memcpy(sendString_u,sendString,64);
  UART_Write(&sendString_u[0],64,UART_A1);

  /*  Send the seconds */
  sprintf(sendString,"%10lu\r\n",seconds);
  memcpy(sendString_u,sendString,64);
  UART_Write(&sendString_u[0],64,UART_A1);
  
}


/** @brief Enters Transmit Iridium Report State
 *
 *  Preps last hour of data to transmit over Iridium
 *  
 *
 *  Format is:
 *      Header -
 *              RAIN YYYY/MM/DD HH:00:00 INTERVAL\r\n
 *      Data -
 *              Mean0 Std0 Mean1 Std1 Mean2 Std2 Mean3 Std4\r\n
 *              Mean5 Std   ...
 *              ...
 *              Mean55 Std55 Mean56 Std56 Mean57 Std57 Mean58 Std58 Mean59 Std59\r\n
 *
 *
 *  @param *Data Hour Data Structure
 *
 *  @return Void
 */
void STATE_TransmitIridium(SampleData_t *Data)
{
  uint16_t year = 0;
  uint8_t mon = 0;
  uint8_t day = 0;
  int8_t currentIdx = 0;
  int8_t stopIdx = 0;
  int8_t idx = 0;
  float mean = 0.0;
  float std = 0.0;
  char line[128] = {0};  
  uint8_t line_u[128] = {0};

  /* Grab the First Time Recorded */
  uint8_t now = 0;
  uint8_t last = 0xFF;
  
  /* Set the index up*/
  currentIdx = HourData.Hour.write;
  currentIdx = currentIdx % 60;
  stopIdx = currentIdx;
  if(stopIdx < 0) {
    stopIdx += 60;
  }
  
  idx = currentIdx;
  /* Retreive buffered data and transmit to UART A1 */
  do
  {
    Buffer8_GetRequested(&HourData.Hour,idx,&now);
    
    if(now==0)
    {
      now = 0xFF;
    }
    
    if(now < last)
    {
      last = now;
      Buffer16_GetRequested(&HourData.Year, idx, &year);
      Buffer8_GetRequested(&HourData.Month,idx,&mon);
      Buffer8_GetRequested(&HourData.Day,idx,&day);
    }
    ++idx;
    idx = idx % 60;
  } while(idx != stopIdx);
  
  sprintf(line,"RAIN %02x/%02x/%04x %02x:00:00 00:01:00 ",mon,day,year,last);
  memcpy(line_u,line,128);
  UART_Write(&line_u[0],LENGTH_OF(line_u),UART_A1);
   __delay_cycles(25000);
   UART_WriteChar(0xff,UART_A1);
  __delay_cycles(25000);
  UART_WriteChar(0xff,UART_A1);
   __delay_cycles(25000);
  UART_WriteChar('\r',UART_A1);
   __delay_cycles(25000);
  UART_WriteChar('\n', UART_A1);
  
  /* Retreive buffered data and transmit to UART A1 */
  uint8_t counter = 5;
  uint8_t endline[] = "\r\n";
  
  /* Set the index up*/
  idx = currentIdx;
  do
  {
    
    if(counter == 0)
    {
      counter = 4;
      UART_Write(&endline[0],LENGTH_OF(endline),UART_A1);
    } else {
      counter--;
    }
   
    /* Retreive the last 60 minutes worth of data from buffers */
    BufferF_GetRequested(&HourData.Mean,idx,&mean);
    BufferF_GetRequested(&HourData.STD,idx,&std);

    
    /* RTC values are in hex, so print hex values to UART*/
    sprintf(line,"%7.2f %7.2f",mean,std);
    memcpy(line_u,line,128);
    UART_Write(&line_u[0],LENGTH_OF(line_u),UART_A1);
    
    ++idx;
    idx = idx % 60;
  } while(idx != stopIdx);
  
  UART_Write(&endline[0],LENGTH_OF(endline),UART_A1);
  UART_Write(&endline[0],LENGTH_OF(endline),UART_A1);
  return;
}


/** @brief Enters Transmit Report State
 *
 *  Preps 60 minutes of data to transmit over UART
 *
 *  Format is:
 *    YYYYMMDD,HH:MM,Mean,STD,Min,Max\r\n
 *    Where:
 *      YYYY - 4-digit Year
 *      MM   - 2-digit Month
 *      DD   - 2-digit Day
 *      HH   - 2-digit Hour (24 hour clock)
 *      MM   - 2-digit Minute
 *      Mean - 2 decimal point, 6 character Mean Volume (mL)
 *      STD - 2 decimal point, 6 character Volume Standard Deviation (mL)
 *      Min - 2 decimal point, 6 character Minimum Volume (mL)
 *      Max - 2 decimal point, 6 character Maximum Volume (mL)
 *
 *  @param volume Volume (in mL)
 *  @param seconds Numer of seconds of integration
 *
 *  @return Void
 */
void STATE_TransmitReport(SampleData_t *Data)
{
  uint16_t year;
  uint8_t mon;
  uint8_t day;
  uint8_t hr;
  uint8_t minute;
  int8_t currentIdx = 0;
  int8_t stopIdx = 0;
  int8_t idx = 0;
  float mean;
  float std;
  float min;
  float max;
  char line[128] = {0};  
  uint8_t line_u[128] = {0};

  /* Set the index up*/
  currentIdx = HourData.Hour.write;
  currentIdx = currentIdx % 60;
  stopIdx = currentIdx;
  if(stopIdx < 0) {
    stopIdx += 60;
  }
  idx = currentIdx;
  /* Retreive buffered data and transmit to UART A1 */
  do
  {
    /* Retreive the last 60 minutes worth of data from buffers */
    Buffer16_GetRequested(&HourData.Year, idx, &year);
    Buffer8_GetRequested(&HourData.Month, idx, &mon);
    Buffer8_GetRequested(&HourData.Day, idx, &day);
    Buffer8_GetRequested(&HourData.Hour, idx, &hr);
    Buffer8_GetRequested(&HourData.Minute, idx, &minute);
    BufferF_GetRequested(&HourData.Mean,idx,&mean);
    BufferF_GetRequested(&HourData.STD,idx,&std);
    BufferF_GetRequested(&HourData.Min,idx,&min);
    BufferF_GetRequested(&HourData.Max,idx,&max);
    
    /* RTC values are in hex, so print hex values to UART*/
    sprintf(line,"@@@%04x%02x%02x,%02x:%02x,%7.2f,%7.2f,%7.2f,%7.2f\r\n",year,mon,day,hr,minute,mean,std,min,max);
    memcpy(line_u,line,128);
    UART_Write(&line_u[0],LENGTH_OF(line_u),UART_A1);
    
    /* Update report index */
    ++idx;
    idx = idx % 60;
  } while(idx != stopIdx);
  
  return;
}

/** @brief Enters Transmit Current Time
 *
 *  Reports the RTC Date & time to user
 *
 *  @return Void
 */
void STATE_TransmitCurrentTime(void)
{
  char OutputStr[64] = {0};
  uint8_t OutputStr_u[64] = {0};
  sprintf(OutputStr, "%04x,%02x,%02x,%02x:%02x:%02x\r\n",RTC.Year,RTC.Mon,RTC.Day,RTC.Hour,RTC.Min,RTC.Sec);
  memcpy(OutputStr_u,OutputStr,64);
  UART_Write(&OutputStr_u[0],LENGTH_OF(OutputStr_u),UART_A1);
  
  return;
}


void ClearBuffers(void)
{
  uint8_t rtcnext = RTCSEC;
  
  
  /* Temporarily Disable GPIO Interrupt */
  GPIO_DetachInputInterrupt(SensorPort, SensorPin);
 
  /* Clear Minute Data Buffers */
  for(uint8_t i=0;i<5;i++) {
    MinuteData.Day[i] = 0;
    MinuteData.Hour[i] = 0;
    MinuteData.Min[i] = 0;
    MinuteData.Mon[i] = 0;
    MinuteData.Year[i] = 0;
    MinuteData.StartIdx[i] = 0;
    MinuteData.EndIdx[i] = 0;
  }
  MinuteData.min = 0;
  MinuteData.sec = 0;
  MinuteData.numSamples = 0;
  MinuteData.lastSampleRecorded = 0;
 
  
  for(uint16_t i=0;i<300;i++) {
    MinuteData.Counts[i] = 0;
  }
  
  /* Clear Hour Data Buffers */
  Buffer16_Clear(&HourData.Year);
  Buffer8_Clear(&HourData.Month);
  Buffer8_Clear(&HourData.Day);
  Buffer8_Clear(&HourData.Hour);
  Buffer8_Clear(&HourData.Minute);
  BufferF_Clear(&HourData.Mean);
  BufferF_Clear(&HourData.STD);
  BufferF_Clear(&HourData.Min);
  BufferF_Clear(&HourData.Max);
  
}

/** @brief Calculates the Volume 
 *
 *  Calculates volume based on formula:
 *  dm = n * 60/tsec
 *  Volume = (A1 /(dm^2)) + A0
 * 
 *  where:
 *  A0 = Sensor Offset
 *  A1 = Sensor Slope
 *  n = Sum of Counts
 *  tsec = Total number of seconds
 *
 *  @param count Frequency count from sensor
 *  @param seconds Number of seconds integrated over
 *
 *  @return Volume
 */
float CalculateVolume(uint32_t count, uint32_t seconds){
  float dm = 0;
  float vol = 0.0;
  
  dm = (float) count * 60;
  dm /= (float) seconds;
 
  /* Look for invalid dms */
  if(( dm > (dmMax) ) || (dm < (dmMin)))
  {
    return NAN;
  }
    
  dm *= dm;
  
  vol = (float) (slope / dm);
  vol += intercept;
  
  if(vol == -0.00)
  {
    vol = 0.0;
  }
  return vol;
  
}

/** @brief Set the system clock
 *
 *  Set the system clock for DCO @ 1MHz, 
 *  SMCLK @500MHSz
 *  MCLK @ 1MHz
 *  ACLK @32.768kHz
 *  LFXT driver on Low Power
 *
 *  @param None
 *
 *  @return Void
 */
void SETUP_Clock(void)
{
  /* Configure the clock                */
  /* DCO running at 8MHz                */
  /* ACLK running on LFXT 32768Hz       */
  /* SMCLK running on DCOCLK, 4MHz    */
  /* MCLK running on DCOCLK, 1MHz       */
  /* LFXT Driver on low power           */
  CSCTL0_H = CSKEY >> 8;		/* Unlock registers */
  CSCTL1 = DCOFSEL_6;			/* Set DCO to 8Mhz */
  CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__2 | DIVM__1;	
  CSCTL4 =   LFXTDRIVE_0 | VLOOFF;
  CSCTL4 &= ~LFXTOFF;
  
  /* Wait for the clock to lock   */
  do
  {
    CSCTL5 &= ~LFXTOFFG;
    SFRIFG1 &= ~OFIFG;
  }while(SFRIFG1 & OFIFG);
  
  /* Lock CS Register */
  CSCTL0_H = 0;							
  
}

/** @brief Configure GPIO
 *
 *  Initialize the GPIO and set an interrupt on P3.0
 *  Set selection bits for UART
 *  Set selection bits for 32kHz crystal
 * 
 *  @param None
 *
 *  @return Void
 */
void SETUP_GPIO(void)
{
  /* Set All GPIO settings to 0 */
  GPIO_Init();        
  
  /* Set up the Capacitance Sensor (counter) Input Pin  */
  GPIO_ClearPin(SensorPort, SensorPin);
  GPIO_SetPinAsInput(SensorPort, SensorPin);
  GPIO_AttachInputInterrupt(SensorPort, SensorPin,GPIO_EDGE_LOW_TO_HIGH);
  
#ifdef DEBUG
  GPIO_ClearPin(DEBUG_PORT,DEBUG_PIN);
  GPIO_SetPinAsOutput(DEBUG_PORT,DEBUG_PIN);
#endif 
  
  /* Configure Selection bits for UART */
  P2SEL1 |= BIT5 | BIT6;                    /* USCI_A0 UART operation */
  P2SEL0 &= ~(BIT5 | BIT6);

  /* LFXIN */
  PJSEL1 &= ~BIT4;
  PJSEL0 |= BIT4;
  
  /* Unlock GPIO */
  PM5CTL0 &= ~LOCKLPM5;		/* Needs to be done after config GPIO & Pins! */
  
  /* Setup RTC Output Pin */
  GPIO_ClearPin(1,0);
  GPIO_SetPinAsOutput(1,0);
  P1SEL1 |= BIT0;
  P1SEL0 &= ~BIT0;
  
   /* Set interrupts */
  P3IFG &= ~BIT0;

  return;
}


#ifdef DEBUG
void BufferTest(void)
{
  
  for(uint8_t i=0;i<59;i++)
  {
    Buffer16_Put(&HourData.Year,i);
    Buffer8_Put(&HourData.Month,i);
    Buffer8_Put(&HourData.Day,i);
    Buffer8_Put(&HourData.Hour,i);
    Buffer8_Put(&HourData.Minute,i);
    BufferF_Put(&HourData.Mean,(i*10.0));
    BufferF_Put(&HourData.STD,(i*1.0));
    BufferF_Put(&HourData.Min,(i*0.1));
    BufferF_Put(&HourData.Max,(i*2.0));
    
  }
  
}
#endif



void STATE_CheckRxBuffer(void)
{
  char value;
  char string[32] = {0};

  
  while((BufferC_IsEmpty(&UartData) == BUFFER_C_NOT_EMPTY) && (SystemState != Offset))
  {
    BufferC_Get(&UartData,&value);
    switch(value)
    {
        case 0x18:
          BufferC_Clear(&UartData);
          break;
        case 'D':
          SystemState = Transmit;
          TxSubState = Volume;
          break;
        case 'd':
          SystemState = Transmit;
          TxSubState = Counts;
          break;
        case 'I':
        case 'i':
          SystemState = Transmit;
          TxSubState = Iridium;
          break;
        case 'R':
        case 'r':
          SystemState = Transmit;
          TxSubState = Report;
          break;
        case 'O':
        case 'o':
          RTC.TimeAtCommand = SecondCounter;
          SystemState = Offset;
          break;
        case 'T':
        case 't':
          /* Added for testing*/
          RTC.Sec = RTCSEC;
          RTC.Min = RTCMIN;
          RTC.Hour = RTCHOUR;
          RTC.Day = RTCDAY;
          RTC.Mon = RTCMON;
          RTC.Year = RTCYEAR;
          SystemState = Transmit; 
          TxSubState = CurrentTime;
          break;
        default:
          SystemState = Sample;
            break;
    }          
  }
  
  if(SystemState == Offset)
  {
//    while(BufferC_HasNewline(&UartData) != BUFFER_C_HAS_NEWLINE)
//    {
//    }
    
    uint8_t idx = 0;
    uint8_t cnt = 0;
    while(idx < 32 && BufferC_IsEmpty(&UartData) == BUFFER_C_NOT_EMPTY)
    {
      BufferC_Get(&UartData,&value);
      if((value >= '0' && value <= '9') || (value == '-'))
      {
        string[cnt] = value;
        cnt++;
      } else if(value == '=' || value == 'o' || value == 'O' )  {

      }else {
        idx = 32;
        UART_WriteNACK(UART_A1);
        
      }
      idx++;
    }
    
    int32_t offsetVal = 0;
    offsetVal = atol(string);

    if(offsetVal != 0)
    {
      UART_WriteACK(UART_A1);
      /* Offset the time */
      RTC_Offset(offsetVal);
      __delay_cycles(10);
    }
    
    
    SystemState = Sample;
    __delay_cycles(10);
  }
}
 

void populatehour(void) {
  
  for(uint8_t i=0;i<58;i++) {
    Buffer16_Put_Circular(&HourData.Year,2016);
    Buffer8_Put_Circular(&HourData.Month,5);
    Buffer8_Put_Circular(&HourData.Day,31);
    Buffer8_Put_Circular(&HourData.Hour,0);
    Buffer8_Put_Circular(&HourData.Minute,i);
    BufferF_Put_Circular(&HourData.Mean,(float)i);
    BufferF_Put_Circular(&HourData.STD,123.2);
    BufferF_Put_Circular(&HourData.Min,0.5);
    BufferF_Put_Circular(&HourData.Max,600.2);
    
  }
  
  RTCHOUR=0;
  RTCMIN=0x47;
  RTCSEC=0x30;
  
}

void populateminute(void) {
  Buffer16_Put_Circular(&HourData.Year,2016);
  Buffer8_Put_Circular(&HourData.Month,6);
  Buffer8_Put_Circular(&HourData.Day,15);
  Buffer8_Put_Circular(&HourData.Hour,0);
  Buffer8_Put_Circular(&HourData.Minute,77);
  BufferF_Put_Circular(&HourData.Mean,(float)RTCSEC);
  BufferF_Put_Circular(&HourData.STD,123.2);
  BufferF_Put_Circular(&HourData.Min,0.5);
  BufferF_Put_Circular(&HourData.Max,600.2);
}


void WD_Init(void) {
  WDTCTL = WDTPW | WDTHOLD | WDTSSEL__ACLK | WDTIS_2 | WDTCNTCL;
  
  return;
}
void WD_Start(void) {
  WDTCTL = WDTPW | WDTSSEL__ACLK | WDTIS_2 | WDTCNTCL;
}
void WD_Kick(void) {
  WDTCTL = WDTPW | WDTSSEL__ACLK | WDTIS_2 | WDTCNTCL;
}


void RestartCounters(void) {
  
MinuteData.Counts[MinuteData.sec] = 0;
FreqPinActive = true;
while(FreqPinActive == true){
  MinuteData.Counts[MinuteData.sec] = 0;
}
SecondCounter = 0;

}