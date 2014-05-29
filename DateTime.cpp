#include "DateTime.h"


DateTime::DateTime()
{
}

void DateTime::Refresh()
{
	RTC_DateTypeDef date;
	RTC_GetDate(RTC_Format_BIN, &date);
	Day = date.RTC_Date;
	Month = date.RTC_Month;
	Dow = date.RTC_WeekDay;
	Year = date.RTC_Year + 2000;

	RTC_TimeTypeDef time;
	RTC_GetTime(RTC_Format_BIN, &time);

	Hours24 = time.RTC_Hours;
	Minutes = time.RTC_Minutes;
	Seconds = time.RTC_Seconds;
}

bool DateTime::Update()
{
	RTC_DateTypeDef date;
	date.RTC_Month = Month;
	date.RTC_Date = Day;
	date.RTC_Year = Year - 2000;
	date.RTC_WeekDay = Dow;
	if (RTC_SetDate(RTC_Format_BIN, &date) != SUCCESS)
		return false;

	RTC_TimeTypeDef time;
	time.RTC_Hours = Hours24;
	time.RTC_Minutes = Minutes;
	time.RTC_Seconds = Seconds;

	return (RTC_SetTime(RTC_Format_BIN, &time) == SUCCESS);
}


DateTime::~DateTime()
{
}
