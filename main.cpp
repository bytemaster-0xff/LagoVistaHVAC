#include <stm32f4xx.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_rcc.h>
#include "Damper.h"
#include "ElectricImp.h"
#include "DebugConsole.h"
#include "DHT22.h"
#include "Timer.h"
#include "misc.h"
#include <stm32f4xx_tim.h>
#include <stm32f4xx_exti.h>
#include "Vent.h"
#include "Command.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "AirHandler.h"

AirHandler *AH;
Damper *Dampers[6];
Vent *Vents[6];
DHT22 *DHTTempSensors [8];
Command *CommandBuffer[COMMAND_BUFFER_LENGTH];
Timer *ScanTimer;
Timer *FlashTimer;

uint8_t CommandBufferHead = 0;
uint8_t CommandBufferTail = 0;

bool _isLedOn = false;
void FlashLED(){
	if (_isLedOn)
		GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_RESET);
	else
		GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_SET);

	_isLedOn = !_isLedOn;
}

void InitMainTimers(){
	RCC_ClockSecuritySystemCmd(ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RCC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 1; //Gives us 0.025ms per tick
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = MAX_COUNT_UP;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM4, ENABLE);

	if (SysTick_Config(SystemCoreClock / 1000))
		while (1);
}

void EnableClocks(){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
}

void PowerDevices(){
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;

	/*Power Up the IMP channel */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_RESET);

	/* Now power on the temp sensors*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);
}


void CreateActionTimers(){
	ScanTimer = new Timer();
	ScanTimer->PeriodMS = 5000;
	ScanTimer->AutoReset = true;
	ScanTimer->CallbackC = ReadAllDHT22;
	ScanTimer->Enable();

	FlashTimer = new Timer();
	FlashTimer->AutoReset = true;
	FlashTimer->CallbackC = FlashLED;
	FlashTimer->PeriodMS = 50;
	FlashTimer->Enable();
}

void ScanCommands(){
	while (CommandBufferHead != CommandBufferTail){
		Command *cmd = CommandBuffer[CommandBufferHead];

		if (strcmp(cmd->Command, "damper") == 0)
			Dampers[atoi(cmd->Args[0]) - 1]->Handle(cmd);
		else if (strcmp(cmd->Command, "ah") == 0)
			AH->Handle(cmd);
		else if (strcmp(cmd->Command, "vent") == 0)
			Vents[atoi(cmd->Args[0]) - 1]->Handle(cmd);
		else
			Imp->Write(cmd->Command);

		Console->Write("\r\n");

		CommandBufferHead++;
		if (CommandBufferHead == COMMAND_BUFFER_LENGTH)
			CommandBufferHead = 0;
	}
}

int main(){
	InitMainTimers();
	EnableClocks();
	PowerDevices();

	for (uint8_t idx = 0; idx < COMMAND_BUFFER_LENGTH; ++idx)
		CommandBuffer[idx] = new Command();

	Console = new DebugConsole();
	Console->Init();

	Imp = new ElectricImp();
	Imp->Init();

	AH = new AirHandler();
	
	Dampers[0] = new Damper(1, GPIOE, GPIO_Pin_12, GPIOE, GPIO_Pin_11);
	Dampers[1] = new Damper(2, GPIOE, GPIO_Pin_10, GPIOE, GPIO_Pin_9);
	Dampers[2] = new Damper(3, GPIOE, GPIO_Pin_8, GPIOE, GPIO_Pin_7);
	Dampers[3] = new Damper(4, GPIOB, GPIO_Pin_2, GPIOB, GPIO_Pin_1);
	Dampers[4] = new Damper(5, GPIOB, GPIO_Pin_0, GPIOC, GPIO_Pin_5);
	Dampers[5] = new Damper(6, GPIOA, GPIO_Pin_4, GPIOA, GPIO_Pin_3);

	Vents[0] = new Vent(1, GPIOD, GPIO_Pin_10, GPIOD, GPIO_Pin_11);
	Vents[1] = new Vent(2, GPIOD, GPIO_Pin_12, GPIOD, GPIO_Pin_13);
	Vents[2] = new Vent(3, GPIOD, GPIO_Pin_14, GPIOD, GPIO_Pin_15);
	Vents[3] = new Vent(4, GPIOC, GPIO_Pin_6, GPIOC, GPIO_Pin_7);
	Vents[4] = new Vent(5, GPIOC, GPIO_Pin_8, GPIOC, GPIO_Pin_9);
	Vents[5] = new Vent(6, GPIOA, GPIO_Pin_8, GPIOA, GPIO_Pin_9);

	Console->Write("Welcome!\r\n");
	SleepMS(1000);
	Imp->Write("NiVek HVAC Controls - ONLINE;");

	DHTTempSensors[0] = new DHT22(101, GPIOC, GPIO_Pin_2, EXTI_Line2, EXTI_PinSource2);
	DHTTempSensors[1] = new DHT22(102, GPIOC, GPIO_Pin_1, EXTI_Line1, EXTI_PinSource1);
	DHTTempSensors[2] = new DHT22(103, GPIOC, GPIO_Pin_0, EXTI_Line0, EXTI_PinSource0);
	DHTTempSensors[3] = new DHT22(104, GPIOD, GPIO_Pin_0, EXTI_Line0, EXTI_PinSource0);
	DHTTempSensors[4] = new DHT22(105, GPIOD, GPIO_Pin_1, EXTI_Line1, EXTI_PinSource1);
	DHTTempSensors[5] = new DHT22(106, GPIOD, GPIO_Pin_2, EXTI_Line2, EXTI_PinSource2);
	DHTTempSensors[6] = new DHT22(107, GPIOD, GPIO_Pin_3, EXTI_Line3, EXTI_PinSource3);
	DHTTempSensors[7] = new DHT22(108, GPIOD, GPIO_Pin_4, EXTI_Line4, EXTI_PinSource4);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
	GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_RESET);

	CreateActionTimers();

	for (;;){
		TimerScan();
		ScanCommands();
	}
}
