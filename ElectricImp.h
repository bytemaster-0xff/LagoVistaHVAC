#pragma once
#include <stm32f4xx_gpio.h>

#define IMP_BUFFER_SIZE 4096

class ElectricImp
{
private:
	uint8_t m_inBuffer[IMP_BUFFER_SIZE];
	uint16_t m_inBufferHead;
	uint16_t m_inBufferTail;

	uint8_t m_outBuffer[IMP_BUFFER_SIZE];
	uint16_t m_outBufferHead;
	uint16_t m_outBufferTail;

	void parseMessage(uint16_t msgEnd);

public:
	ElectricImp();
	~ElectricImp();

	void Init();
	void Write(const char *msg);
	void HandleCh(char ch);
	void SendNextCh();
	void HandleOverrun();
};

extern ElectricImp *Imp;

