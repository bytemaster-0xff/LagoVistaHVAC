#include "DHT22.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_rcc.h"
#include <misc.h>
#include <stdio.h>
#include "stm32f4xx_tim.h"
#include "stm32f4xx_syscfg.h"
#include "ElectricImp.h"
#include "DebugConsole.h"

#define NULL 0

static DHT22 *pHead = NULL;
static DHT22 *pCurrent = NULL;

extern "C" void ReadAllDHT22(){
	pCurrent = pHead;
	if (pCurrent != NULL)
		pCurrent->Update();
}

extern "C" void ReadNextDHT22() {
	if (pCurrent == NULL){
		Imp->Write("OOPS!");
	}
	else{
		pCurrent = pCurrent->pNext;
		if (pCurrent != NULL)
			pCurrent->Update();
		else
			Console->Write("ALL DONE");
	}
}
 
extern "C" void DH22HandleTransition(uint32_t pr, uint32_t msk){
	if (pCurrent != NULL)
		pCurrent->HandleTransition(pr, msk);
}

DHT22::DHT22(uint8_t id, GPIO_TypeDef* port, uint16_t pin, uint32_t extiPin, uint32_t pinSource){
	m_id = id;
	m_port = port;
	m_pin = pin;
	m_extiPin = extiPin;
	m_pinSource = pinSource;
	m_temperature = 0;
	m_humidity = 0;
	m_isOnline = 0;
	pNext = NULL;

	m_outputConfig.GPIO_Pin = pin;
	m_outputConfig.GPIO_Mode = GPIO_Mode_OUT;
	m_outputConfig.GPIO_Speed = GPIO_Speed_50MHz;
	m_outputConfig.GPIO_OType = GPIO_OType_PP;
	m_outputConfig.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(m_port, &m_outputConfig);
	/* Keep High, first thing to do is bring low for a bit... */
	GPIO_WriteBit(m_port, m_pin, Bit_SET);

	m_inputConfig.GPIO_Pin = pin;
	m_inputConfig.GPIO_Mode = GPIO_Mode_IN;
	m_inputConfig.GPIO_Speed = GPIO_Speed_50MHz;
	m_inputConfig.GPIO_OType = GPIO_OType_PP;
	m_inputConfig.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_InitTypeDef GPIO_InitStructure;
	
	m_state = DHTIdle;

	if (port == GPIOA){
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		m_portSource = EXTI_PortSourceGPIOA;
	}
	else if (port == GPIOB){
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		m_portSource = EXTI_PortSourceGPIOB;
	}
	else if (port == GPIOC){
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
		m_portSource = EXTI_PortSourceGPIOC;
	}
	else if (port == GPIOD){
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
		m_portSource = EXTI_PortSourceGPIOD;
	}
	else if (port == GPIOE){
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		m_portSource = EXTI_PortSourceGPIOE;
	}

	m_pTimer = new Timer();
	m_pTimer->AutoReset = 0;
	m_pTimer->PeriodMS = 2;	
	m_pTimer->Callback = new TCEventHandler<DHT22>(this, &DHT22::donePause);

	m_pTimeoutTimer = new Timer();
	m_pTimeoutTimer->AutoReset = 0;
	m_pTimeoutTimer->PeriodMS = 50;
	m_pTimeoutTimer->Callback = new TCEventHandler<DHT22>(this, &DHT22::timeout);

	NVIC_InitTypeDef nvicInit;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 0x00; /* 0 = Highest, 3= Lowest */
	nvicInit.NVIC_IRQChannelSubPriority = 0;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;

	if (m_pinSource == EXTI_PinSource0) nvicInit.NVIC_IRQChannel = EXTI0_IRQn;
	if (m_pinSource == EXTI_PinSource1) nvicInit.NVIC_IRQChannel = EXTI1_IRQn;
	if (m_pinSource == EXTI_PinSource2) nvicInit.NVIC_IRQChannel = EXTI2_IRQn;
	if (m_pinSource == EXTI_PinSource3) nvicInit.NVIC_IRQChannel = EXTI3_IRQn;
	if (m_pinSource == EXTI_PinSource4) nvicInit.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_Init(&nvicInit);

	if (pHead == NULL)
		pHead = this;
	else{
		DHT22 *pDHT22 = pHead; 
		while (pDHT22->pNext != NULL)
			pDHT22 = pDHT22->pNext;

		pDHT22->pNext = this;
	}
}

void DHT22::timeout(void){
	disableIRQ();

	m_state = DHTIdle;
	m_isOnline = false;
	m_errCode = 3;

	GPIO_Init(m_port, &m_outputConfig);
	GPIO_WriteBit(m_port, m_pin, Bit_SET);

	char msg[30];
	sprintf(msg, "dht22,%d,offline,3;", m_id);
	Imp->Write(msg);

	ReadNextDHT22();
}

void DHT22::donePause(void){	
	m_edgeCount = 0;
	m_state = DHTExpectInitialResponse_Low;
	GPIO_WriteBit(m_port, m_pin, Bit_SET);
	GPIO_Init(m_port, &m_inputConfig);
	enableIRQ();
}

void DHT22::enableIRQ(void) {
	EXTI_InitTypeDef extInit;
	extInit.EXTI_Line = m_extiPin;
	extInit.EXTI_Mode = EXTI_Mode_Interrupt;
	extInit.EXTI_LineCmd = ENABLE;
	extInit.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&extInit);
	SYSCFG_EXTILineConfig(m_portSource, m_pinSource);

}

void DHT22::disableIRQ(void){
	EXTI_InitTypeDef extInit;
	extInit.EXTI_Line = m_extiPin;
	extInit.EXTI_Mode = EXTI_Mode_Interrupt;
	extInit.EXTI_LineCmd = DISABLE;
	extInit.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&extInit);
}

void DHT22::finalizeReading() {
	disableIRQ();

	m_state = DHTIdle;

	uint8_t cs = 0;
	for (int byteIndex = 0; byteIndex < 4; ++byteIndex)
		cs += m_inBuffer[byteIndex];

	if (cs == m_inBuffer[4]){
		m_temperature = (((m_inBuffer[2] << 8 | m_inBuffer[3]) * 9) / 5) + 320;
		m_humidity = m_inBuffer[0] << 8 | m_inBuffer[1];
		m_errCode = 0;
		m_isOnline = true;
	
		char msg[50];
		float fTemp = m_temperature / 10.0f;
		float fHumid = m_humidity / 10.0f;
		sprintf(msg, "dht22,%d,ok,%0.1f,%0.1f;", m_id, fTemp, fHumid);
		Imp->Write(msg);
	}
	else{
		m_isOnline = false;
		m_errCode = 2;

		char msg[30];
		float fTemp = m_temperature / 10.0f;
		float fHumid = m_humidity / 10.0f;
		sprintf(msg, "dht22,%d,err,2;", m_id);
		Imp->Write(msg);
	}

	m_pTimeoutTimer->Disable();

	GPIO_Init(m_port, &m_outputConfig);
	GPIO_WriteBit(m_port, m_pin, Bit_SET);

	ReadNextDHT22();
}

void DHT22::Update() {
	m_pTimeoutTimer->Enable();

	for (uint8_t idx = 0; idx < 5; ++idx)
		m_inBuffer[idx] = 0;

	GPIO_Init(m_port, &m_outputConfig);

	GPIO_WriteBit(m_port, m_pin, Bit_RESET);

	m_pTimer->Enable();
}

void DHT22::HandleTransition(uint32_t pr, uint32_t msk){
	if (((pr & m_extiPin) != (uint32_t) RESET) && ((msk & m_extiPin) != (uint32_t) RESET)){
		uint8_t status = ((m_port->IDR & m_pin) == m_pin) ? SET : RESET;

		switch (m_state){
			case DHTIdle: break;
			case DHTExpectInitialResponse_Low:
				if (status == RESET)
					m_state = DHTExpectInitial_High;
				break;
			case DHTExpectInitial_High:
				m_state = DHTExpectInitial_Low;
				break;
			case DHTExpectInitial_Low:
				m_state = DHTExpectData_Start_Data_High;
				m_readBitIndex = 0;
				m_readByteIndex = 0;
				break;

			case DHTExpectData_Start_Data_High:
				m_startReading = TIM4->CNT;
				m_state = DHTExpectData_End_Data_low;
				break;

			case DHTExpectData_End_Data_low:
				++m_edgeCount;
				m_endReading = TIM4->CNT;
				m_deltaReading = m_endReading + ((m_startReading > m_endReading) ? MAX_COUNT_UP : 0) - m_startReading;

				//Shift previous bytes over.
				m_inBuffer[m_readByteIndex] <<= 1;

				//If more than 1200 we have a one so set the LSB to 1
				if (m_deltaReading > 1200)
					m_inBuffer[m_readByteIndex] |= 1;				

				if (++m_readBitIndex == 8){
					m_readBitIndex = 0;
					m_readByteIndex++;
				}

				if (m_readByteIndex == 5)
					finalizeReading();
				else
					m_state = DHTExpectData_Start_Data_High;

				break;
		}
	}
}


DHT22::~DHT22()
{
}


