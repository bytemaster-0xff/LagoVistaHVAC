#pragma once

#include "Timer.h"
#include "Command.h"
#include <stm32f4xx_gpio.h>

class Vent
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

	GPIO_TypeDef* m_portIn1;
	uint16_t m_pinIn1;
	GPIO_TypeDef* m_portIn2;
	uint16_t m_pinIn2;
	
	Timer *m_pTimer;
	States	m_currentState;

	GPIO_InitTypeDef inputPinState;
	GPIO_InitTypeDef outputPinState;


public:
	Vent(uint8_t id, GPIO_TypeDef* portIn1, uint16_t pinIn1,
		GPIO_TypeDef* portIn2, uint16_t pinIn2);

	const char *GetCurrentState();

	void TransitionCompleted();
	void SetTransitionTime(uint16_t ms);
	void Handle(Command *cmd);
	void Open();
	void Close();
};

