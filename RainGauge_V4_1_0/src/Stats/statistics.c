/** @file statistics.c
 *  @brief Statistics functions
 *
 *  @author Matt Casari, matthew.casari@noaa.org
 *  @date March 23, 2016
 *  @version 0.1.0
 *
 *  @copyright National Oceanic and Atmospheric Administration
 *  @copyright Pacific Marine Environmental Lab
 *  @copyright Environmental Development Division
 *
 *	@note Tested on MSP430FR5969
 *
 *  @bug  No known bugs
 */
#include "statistics.h"

/************************************************************************
*						GLOBAL FUNCTIONS
************************************************************************/
uint8_t STATS_CalculateMean(float *vals, uint16_t length, float *mean)
{
	float sum = 0.0;
	float CalculatedMean = 0.0;
    uint16_t nanCnt = 0;
	uint16_t i = 0;
	
    /* Look for lengths > 0 or fail */
	if(length <1)
 	{
      *mean = NAN;
      return STATISTICS_FAIL;
 	}
 	
    /* Sum all values (other than nan) together) */
	for(i=0;i<length;i++)
	{
      if (isnan(vals[i])){
        nanCnt++;
      }
      else {
        sum += vals[i];
      }
		
	}
    
    /* If all values are NAN, fail, otherwise calculate mean */
    if(nanCnt == length)
    {
      CalculatedMean = NAN;
    } else {
      
      CalculatedMean = (sum/(length - nanCnt));
    }
    
	*mean = CalculatedMean;

	return STATISTICS_OK;	
}


uint8_t STATS_FindMax(float *vals, uint16_t length, float *max)
{
 	float MaxVal = 0;
 	uint16_t i = 0;
 	
    /* Look for lengths > 0 or fail */
 	if(length <1)
 	{
 		return STATISTICS_FAIL;
 	}
 	
 	MaxVal = 0;
 	
    /* Search array for Max value */
 	for(i=0;i<length;i++)
 	{
      if(!isnan(vals[i])){
 		if(vals[i]>MaxVal)
 		{
 			MaxVal = vals[i];
 		}
      }
 	}
 	
 	*max = MaxVal;
 	
 	
 	return STATISTICS_OK;
}


uint8_t STATS_FindMin(float *vals, uint16_t length, float *min)
{
 	float MinVal = 0;
 	uint16_t i = 0;
 	
    /* Look for lengths > 0 or fail */
 	if(length <1)
 	{
 		return STATISTICS_FAIL;
 	}
 	
    
 	MinVal = 999.9;
 	/* Search array for min value */
 	for(i=0;i<length;i++)
 	{
      if(!isnan(vals[i])){
 		if(vals[i]<MinVal)
 		{
 			MinVal = vals[i];
 		}
      }
 	}
 	
    if(MinVal == 999.9)
    {
      MinVal = 0;
    }
    
 	*min = MinVal;
 	
 	
 	return STATISTICS_OK;
}


uint8_t STATS_ComputeSTD(float *vals, uint16_t length, float mean, float *std)
{
	float VarianceSum = 0;
	float Diff = 0;
    uint16_t nanCnt = 0;
	uint16_t i;
    
    /* Look for lengths > 0 or fail */
	if(length <= 0)
	{
		return STATISTICS_FAIL;	
	}
	
    /* Calculate the variance, ignoring bad values */
	for(i=0;i<length;i++)
	{
      if(isnan(vals[i])){
        nanCnt++;
      } else {
		Diff = vals[i] - mean;
		Diff *= Diff;
		VarianceSum += Diff;
      }
		
	}
	VarianceSum /= (length - nanCnt);
	
    /* Take sqrt to find STD */
	*std = sqrt(VarianceSum);
	
	return STATISTICS_OK;
}
