uint8_t RTC_Offset(uint32_t offset) {
	int32_t JulianSecond = 0;
	int32_t JulianMinute = 0;
	int32_t JulianHour = 0;
	int32_t JulianDay = 0;
	uint16_t Year = RTC.Year;
	uint8_t Mon = RTC.Mon;
	uint8_t Day = RTC.Day;
	uint8_t Hour = RTC.Hour;
	uint8_t Min = RTC.Min;
	uint8_t Sec = RTC.Sec;
	int32_t temp;
	
	if((offset > 31536000) || (offset < -31536000)) {
		return RTC_BAD_OFFSET;
	}
	
	/* Calculate the Juilan Day */
	for(uint8_t i=0;i<Mon;i++)
	{
		JulianDay += GetDaysInMonth(i,Year); 
	}
	
	/* Convert to get the Juilan Second */
	JulianSecond = JulianDay * 86400;		/* 86400 seconds in a day */
	JulianSecond += (Hour * 3600) 			/* 3600 Seconds in an hour */
	JulianSecond += (Min * 60)					/* 60 seoncs in a minute */
	JulianSecond += Sec;
	
	/* Add in the offset */
	JulianSecond += offset;
	
	/* Did we lose a year? */
	if(JulianSecond < 0) {
		Year -= 1;
		
		/* Look for leap year then adjust seconds */
		if((Year % 4) == 0) {
			JulianSecond = 31622400 + JulianSecond;
		} else {
			JulianSecond = 31536000 + JulianSecond;
		}
	}
	
	/* Calculate the Julian Minute, Hour & Day with offset */
	JulianMinute = JulianSecond / 60;
	JulianHour = JulianMinute / 60;
	JulianDay = JulianHour / 24;
	
	
	Sec = (uint8_t) (JulianSecond % 60);
	Min = (uint8_t) (JulianMinute % 60);
	Hour = (uint8_t) (JulianHour % 24);
	
	Mon = 1;
	uint8_t NumDaysInMonth = 0;
	for(uint8_t i=1;i<13;i++) {
		NumDaysInMonth = GetDaysInMonth(idx,Year);
		
		if(JulianDay > NumDaysInMonth)
		{
			JulianDay -= NumDaysInMonth;
			Mon++;
		} else {
			Day = (uint8_t) JulianDay;
			i=13;
		}
		
	}
	
	
	RTC.Year = Year;
	RTC.Mon = Mon;
	RTC.Day = Day;
	RTC.Hour = Hour;
	RTC.Min = Min;
	RTC.Sec = Sec;

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
		case 10:
		case 12:
			days = 31;
			break;
		default:
			break;
	}
	
	return days;
}