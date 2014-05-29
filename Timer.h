#pragma once

#include <stm32f4xx.h>
#include "TEventHandler.h"
#define NULL 0

class Timer
{
private:

public:
	Timer();
	~Timer();

	bool AutoReset;
	bool IsEnabled;
	bool IsExpired;

	uint32_t PeriodMS;
	uint32_t CurrentMS;

	Timer *pNext;

	void Enable();
	void Disable();

	TEventHandler *Callback;
	void (*CallbackC)(void);
};

extern "C" void TimerUpdate();
extern "C" void TimerScan();
extern "C" void SleepMS(uint32_t ms);
