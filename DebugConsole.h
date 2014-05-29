#pragma once
#include <stm32f4xx_gpio.h>

#define DEBUG_BUFFER_SIZE 1024

class DebugConsole
{
private:
	uint8_t m_debugBuffer[DEBUG_BUFFER_SIZE];
	uint16_t m_debugBufferHead;
	uint16_t m_debugBufferTail;

public:
	DebugConsole();
	~DebugConsole();

	void Write(const char *);
	void HandleCh(char ch);
	void SendNextCh();
	void HandleOverrun();
	void Init();
};

extern DebugConsole *Console;

