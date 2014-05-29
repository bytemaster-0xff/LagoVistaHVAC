#include "DebugConsole.h"
#include "ElectricImp.h"
#include "DHT22.h"
#include <stm32f4xx_exti.h>
#include "Timer.h"

extern "C" void USART3_IRQHandler(){
	if ((USART3->SR & USART_SR_RXNE) == USART_SR_RXNE)
		Imp->HandleCh(USART3->DR);
	else if ((USART3->SR & USART_SR_TXE) == USART_SR_TXE)
		Imp->SendNextCh();
	else if ((USART3->SR & USART_SR_ORE) == USART_SR_ORE)
		Imp->HandleOverrun();
}

extern "C" void UART4_IRQHandler(){
	if ((UART4->SR & USART_SR_RXNE) == USART_SR_RXNE)
		Console->HandleCh(UART4->DR);
	else if ((UART4->SR & USART_SR_TXE) == USART_SR_TXE)
		Console->SendNextCh();
	else if ((UART4->SR & USART_SR_ORE) == USART_SR_ORE)
		Console->HandleOverrun();
}

extern "C" void SysTick_Handler(void){
	TimerUpdate();
}


void Delay100MS()
{
	int i;
	for (i = 0; i < 1000000; i++);
}

void sendHardFaultToImp(){
	const char *hardFault = "hardfault;";
	
	char *ch = (char*) hardFault;
	
	while (*ch != 0x00){
		USART3->DR = *ch;
		while ((USART3->SR & USART_SR_TXE) != USART_SR_TXE);
		ch++;
	}
}

extern "C" void HardFault_Handler(void) {
	sendHardFaultToImp();
	while (true){
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
		Delay100MS();
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
		Delay100MS();
	}
}

extern "C" void EXTI0_IRQHandler(){
	DH22HandleTransition(EXTI->PR, EXTI->IMR);

	EXTI_ClearITPendingBit(EXTI->PR);
}

extern "C" void EXTI1_IRQHandler()   {
	DH22HandleTransition(EXTI->PR, EXTI->IMR);

	EXTI_ClearITPendingBit(EXTI->PR);
}

extern "C" void EXTI2_IRQHandler(){
	DH22HandleTransition(EXTI->PR, EXTI->IMR);

	EXTI_ClearITPendingBit(EXTI->PR);
}

extern "C" void EXTI3_IRQHandler(){
	DH22HandleTransition(EXTI->PR, EXTI->IMR);

	EXTI_ClearITPendingBit(EXTI->PR);
}

extern "C" void EXTI4_IRQHandler(void){
	DH22HandleTransition(EXTI->PR, EXTI->IMR);

	EXTI_ClearITPendingBit(EXTI->PR);
}

extern "C" void EXTI9_5_IRQHandler(void){
	DH22HandleTransition(EXTI->PR, EXTI->IMR);

	EXTI_ClearITPendingBit(EXTI->PR);
}

extern "C" void EXTI15_10_IRQHandler(void){
	DH22HandleTransition(EXTI->PR, EXTI->IMR);

	EXTI_ClearITPendingBit(EXTI->PR);
}

