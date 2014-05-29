#include "Timer.h"

Timer *pHead = NULL;

uint32_t __sleepTimer = 0;

void TimerScan(){
	Timer *ptr = pHead;

	while (ptr != NULL){
		if (ptr->IsEnabled &&        /* If we are enabled */
			!ptr->IsExpired &&        /* And haven't already expired */
			ptr->CurrentMS == 0){    /* And current MS just went to zero */
			ptr->IsExpired = true;

			if (ptr->Callback != NULL)
				ptr->Callback->Call();  /* Execute the handler */
			
			if (ptr->CallbackC != NULL)
				ptr->CallbackC();

			if (ptr->AutoReset){
				ptr->CurrentMS = ptr->PeriodMS;
				ptr->IsExpired = false;
			}
			else
				ptr->IsExpired = true;    /* Otherwise mark as expired */
		}

		ptr = ptr->pNext;
	}
}

/* Called once per second on IRQ handler. */
extern "C" void TimerUpdate()
{
	if (__sleepTimer > 0)
		__sleepTimer--;

	Timer *ptr = pHead;
	while (ptr != NULL){
		if (ptr->IsEnabled && ptr->CurrentMS > 0)
			ptr->CurrentMS--;

		ptr = ptr->pNext;
	}
}

/* Forces a sleep */
extern "C" void SleepMS(uint32_t sleepMS){
	__sleepTimer = sleepMS;
	while (__sleepTimer > 0);
}

Timer::Timer(){
	pNext = NULL;
	Callback = NULL;
	CallbackC = NULL;
	IsExpired = false;
	IsEnabled = false;

	if (pHead == NULL)
		pHead = this;
	else{
		Timer *ptr = pHead;
		while (ptr->pNext != NULL)
			ptr = ptr->pNext;

		ptr->pNext = this;
	}
}

void Timer::Enable()
{
	CurrentMS = PeriodMS;
	IsEnabled = true;
	IsExpired = false;
}

void Timer::Disable(){
	CurrentMS = 0;
	IsEnabled = false;
	IsExpired = false;
}

Timer::~Timer()
{
}
