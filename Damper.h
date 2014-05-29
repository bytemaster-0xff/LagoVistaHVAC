#pragma once

#include "stm32f4xx_gpio.h"
#include "Timer.h"
#include "Command.h"

class Damper
{
public:
	typedef enum e_CurrentStates
	{
		Unknown = -1,
		Opened = 10,
		Opening = 11,
		Closed = 20,
		Closing = 21
	}  States;

private: 
	uint16_t m_transitionMS;
	uint8_t m_id;
	GPIO_TypeDef* m_portOpen;
	uint16_t m_pinOpen;
	GPIO_TypeDef* m_portClose;
	uint16_t m_pinClose;
	Timer *m_pTimer;
	States		m_currentState;

public:
	Damper(uint8_t id, GPIO_TypeDef* portOpen, uint16_t pinOpen,
	 	   GPIO_TypeDef* portClose, uint16_t pinClose);

	const char *GetCurrentState();

	void TransitionCompleted();
	void SetTransitionTime(uint16_t ms);
	void Handle(Command *cmd);
	void Open();
	void Close();
};

