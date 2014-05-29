#pragma once

#include <stm32f4xx_rtc.h>

class DateTime
{
public:
	uint8_t Month;
	uint8_t Day;
	uint16_t Year;
	uint8_t Dow;

	uint8_t Hours24;
	uint8_t Minutes;
	uint8_t Seconds;

	DateTime();
	void Refresh();
	bool Update();

	~DateTime();
};

