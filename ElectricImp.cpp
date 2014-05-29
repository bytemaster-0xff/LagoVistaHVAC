#include "ElectricImp.h"
#include "DebugConsole.h"
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <misc.h>
#include <stdio.h>
#include <string.h>
#include "Command.h"

ElectricImp *Imp;


ElectricImp::ElectricImp()
{
	m_inBufferHead = 0;
	m_inBufferTail = 0;
	m_outBufferHead = 0;
	m_outBufferTail = 0;
}


void ElectricImp::Init(){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	NVIC_InitTypeDef nvicInit;
	nvicInit.NVIC_IRQChannel = USART3_IRQn;

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

	USART_Init(USART3, &USART_InitStructure);

	USART_Cmd(USART3, ENABLE);
}

void ElectricImp::Write(const char *msg)
{
	while (*msg){
		m_outBuffer[m_outBufferTail++] = *msg;
		if (m_outBufferTail == IMP_BUFFER_SIZE)
			m_outBufferTail = 0;

		msg++;
	}

	if (m_outBufferHead != m_outBufferTail)
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}

/* Very Simple Protocol 
CMD,ARG1,ARG2,ARG3,ARG4,ARG5
Can have a maximum of 5 args
To save malloc statements we 
will have a command buffer of
5 commands.
Each 

*/



void ElectricImp::parseMessage(uint16_t tail){
	uint8_t argIdx = 255; /* Start out at -1, this will set chars to cmd*/
	uint8_t chIdx = 0;

	Command *cmd = CommandBuffer[CommandBufferTail];

	while (m_inBufferHead != tail){
		char ch = m_inBuffer[m_inBufferHead];
		if (ch == ','){
			argIdx++;
			chIdx = 0;
		}
		else{

			if (argIdx == 255){
				cmd->Command[chIdx++] = ch;
				cmd->Command[chIdx] = 0x00;
			}
			else{
				cmd->Args[argIdx][chIdx++] = ch;
				cmd->Args[argIdx][chIdx] = 0x00;
			}
		}

		m_inBufferHead++;
		if (m_inBufferHead == IMP_BUFFER_SIZE)
			m_inBufferHead = 0;
	}

	cmd->NumberArgs = argIdx + 1;

	Console->Write("SHOULD HANDLE => ");
	Console->Write(cmd->Command);
	Console->Write("\r\n");

	//Got it so now mark as ready.
	CommandBufferTail++;
	if (CommandBufferTail == COMMAND_BUFFER_LENGTH)
		CommandBufferTail = 0;
}
 
void ElectricImp::HandleCh(char ch){
	
	if (ch == ';')
		this->parseMessage(m_inBufferTail);
	else
		m_inBuffer[m_inBufferTail++] = ch;

	if (m_inBufferTail == IMP_BUFFER_SIZE)
		m_inBufferTail = 0;

}

void ElectricImp::SendNextCh()
{
	USART3->DR = m_outBuffer[m_outBufferHead++];
	if (m_outBufferHead == IMP_BUFFER_SIZE)
		m_outBufferHead = 0;

	if (m_outBufferHead == m_outBufferTail)
		USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
}

void ElectricImp::HandleOverrun()
{

}

ElectricImp::~ElectricImp()
{
}
