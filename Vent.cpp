#include "Vent.h"
#include <string.h>
#include <stm32f4xx_gpio.h>
#include "ElectricImp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


Vent::Vent(uint8_t id, GPIO_TypeDef* portIn1, uint16_t pinIn1,
	GPIO_TypeDef* portIn2, uint16_t pinIn2)
{
	//m_transitionMS = 250;
	m_id = id;
	m_portIn1 = portIn1;
	m_portIn2 = portIn2;

	m_pinIn1 = pinIn1;
	m_pinIn2 = pinIn2;

	outputPinState.GPIO_Mode = GPIO_Mode_OUT;
	outputPinState.GPIO_Speed = GPIO_Speed_50MHz;

	inputPinState.GPIO_Mode = GPIO_Mode_OUT;
	inputPinState.GPIO_Speed = GPIO_Speed_50MHz;

	inputPinState.GPIO_Pin = m_pinIn1;
	GPIO_Init(m_portIn1, &inputPinState);

	inputPinState.GPIO_Pin = m_pinIn2;
	GPIO_Init(m_portIn2, &inputPinState);

	m_pTimer = new Timer();
	m_pTimer->AutoReset = false;
	m_pTimer->PeriodMS = 500;
	m_pTimer->Callback = new TCEventHandler<Vent>(this, &Vent::TransitionCompleted);
}

void Vent::Handle(Command *cmd)
{
	/* Args 0 will contain the damper #*/
	if (strcmp(cmd->Args[1], "open") == 0)
		Open();
	else if (strcmp(cmd->Args[1], "close") == 0)
		Close();
	else if (strcmp(cmd->Args[1], "ms"))
		m_transitionMS = atoi(cmd->Args[2]);
}

const char * Vent::GetCurrentState(){
	switch (m_currentState){
		case Opened: return "opened";
		case Closed: return "closed";
		case Opening: return "opening";
		case Closing: return "closing";
		default: return "uknown";
	}
}

void Vent::Open() {
	outputPinState.GPIO_Pin = m_pinIn1;
	GPIO_Init(m_portIn1, &outputPinState);
	GPIO_WriteBit(m_portIn1, m_pinIn1, Bit_RESET);

	outputPinState.GPIO_Pin = m_pinIn2;
	GPIO_Init(m_portIn2, &outputPinState);
	GPIO_WriteBit(m_portIn2, m_pinIn2, Bit_SET);

	m_currentState = Opening; 
	m_pTimer->Enable();
}

void Vent::TransitionCompleted() {
	outputPinState.GPIO_Pin = m_pinIn1;
	GPIO_Init(m_portIn1, &outputPinState);
	GPIO_WriteBit(m_portIn1, m_pinIn1, Bit_RESET);
	GPIO_WriteBit(m_portIn2, m_pinIn2, Bit_RESET);

	m_pTimer->Disable();
	/*inputPinState.GPIO_Pin = m_pinIn1;
	GPIO_Init(m_portIn1, &inputPinState);

	inputPinState.GPIO_Pin = m_pinIn2;
	GPIO_Init(m_portIn2, &inputPinState);
	*/
	if (m_currentState == Opening){
		m_currentState = Opened;
		char msg[20];
		sprintf(msg, "vent,%d,opened;", m_id);
		Imp->Write(msg);
	}
	else if (m_currentState == Closing){
		m_currentState = Closed;
		char msg[20];
		sprintf(msg, "vent,%d,closed;", m_id);
		Imp->Write(msg);
	}
}

void Vent::SetTransitionTime(uint16_t ms){
	m_transitionMS = ms;
}

void Vent::Close() {
	outputPinState.GPIO_Pin = m_pinIn1;
	GPIO_Init(m_portIn1, &outputPinState);
	GPIO_WriteBit(m_portIn1, m_pinIn1, Bit_SET);

	outputPinState.GPIO_Pin = m_pinIn2;
	GPIO_Init(m_portIn2, &outputPinState);
	GPIO_WriteBit(m_portIn2, m_pinIn2, Bit_RESET);

	m_currentState = Closing;
	m_pTimer->Enable();
}

