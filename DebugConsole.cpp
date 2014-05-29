#include "DebugConsole.h"
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <misc.h>

DebugConsole *Console;

DebugConsole::DebugConsole()
{
	m_debugBufferHead = 0;
	m_debugBufferTail = 0;
}

void DebugConsole::Init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

	NVIC_InitTypeDef nvicInit;
	nvicInit.NVIC_IRQChannel = UART4_IRQn;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInit);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(UART4, &USART_InitStructure);

	USART_Cmd(UART4, ENABLE);
}

void DebugConsole::HandleCh(char ch)
{

}

void DebugConsole::Write(const char *msg){
	while (*msg)
	{
		m_debugBuffer[m_debugBufferTail++] = *msg;
		if (m_debugBufferTail == DEBUG_BUFFER_SIZE)
			m_debugBufferTail = 0;

		msg++;
	}
	
	if (m_debugBufferHead != m_debugBufferTail)
		USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
}

void DebugConsole::SendNextCh()
{
	if (m_debugBufferHead == m_debugBufferTail){
		USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
		return;
	}

	UART4->DR = m_debugBuffer[m_debugBufferHead++];
	if (m_debugBufferHead == DEBUG_BUFFER_SIZE)
		m_debugBufferHead = 0;
}

void DebugConsole::HandleOverrun()
{

}

DebugConsole::~DebugConsole()
{
}
