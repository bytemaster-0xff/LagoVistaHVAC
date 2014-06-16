#include "Damper.h"
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include "ElectricImp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DebugConsole.h"


Damper::Damper(uint8_t id, GPIO_TypeDef* portOpen, uint16_t pinOpen,
			   GPIO_TypeDef* portClose, uint16_t pinClose){
	m_id = id;
	m_transitionMS = 15000;
	m_portOpen = portOpen;
	m_portClose = portClose;

	m_pinOpen = pinOpen;
	m_pinClose = pinClose;

	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = pinOpen;
	GPIO_Init(portOpen, &GPIO_InitStructure);
	GPIO_WriteBit(portOpen, pinOpen, Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = pinClose;
	GPIO_Init(portClose, &GPIO_InitStructure);

	GPIO_WriteBit(portClose, pinClose, Bit_RESET);
	
	m_pTimer = new Timer();
	m_pTimer->AutoReset = false;
	m_pTimer->Callback = new TCEventHandler<Damper>(this, &Damper::TransitionCompleted);
}

void Damper::Handle(Command *cmd){
	/* Args 0 will contain the damper #*/
	if (strcmp(cmd->Args[1], "open") == 0)
		Open();
	else if (strcmp(cmd->Args[1], "close") == 0)
		Close();
	else if (strcmp(cmd->Args[1], "ms"))
		m_transitionMS = atoi(cmd->Args[2]);
}

const char *Damper::GetCurrentState(){
	switch (m_currentState){
		case Opened: return "opened";
		case Closed: return "closed";
		case Opening: return "opening";
		case Closing: return "closing";
		default: return "uknown";
	}
}

void Damper::Open() {
	if (m_currentState == Closing)
		GPIO_WriteBit(m_portClose, m_pinClose, Bit_RESET);

	m_pTimer->PeriodMS = m_transitionMS;
	GPIO_WriteBit(m_portOpen, m_pinOpen, Bit_SET);
	m_currentState = Opening;
	m_pTimer->Enable();
}

void Damper::TransitionCompleted() {
	if (m_currentState == Opening){
		GPIO_WriteBit(m_portOpen, m_pinOpen, Bit_RESET);
		m_currentState = Opened;
		char msg[20];
		sprintf(msg, "damper,%d,opened;", m_id);
		Imp->Write(msg);
	}
	else if (m_currentState == Closing){
		GPIO_WriteBit(m_portClose, m_pinClose, Bit_RESET);
		m_currentState = Closed;
		char msg[20];
		sprintf(msg, "damper,%d,closed;", m_id);
		Imp->Write(msg);
	}
}

void Damper::SetTransitionTime(uint16_t ms){
	m_transitionMS = ms;
}

void Damper::Close() {
	if (m_currentState == Opening)
		GPIO_WriteBit(m_portOpen, m_pinOpen, Bit_RESET);
	  
	m_pTimer->PeriodMS = m_transitionMS;
	GPIO_WriteBit(m_portClose, m_pinClose, Bit_SET);
	m_currentState = Closing;
	m_pTimer->Enable();
}