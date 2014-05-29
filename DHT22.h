#pragma once

#include "stm32f4xx_gpio.h"
#include "Timer.h"

#define MAX_COUNT_UP 20000

class DHT22
{
private: 
	uint8_t m_id;
	GPIO_TypeDef* m_port;
	uint16_t m_pin;
	uint16_t m_edgeCount;

	uint8_t m_pinSource;
	uint8_t m_portSource;

	uint16_t m_readBitIndex;
	uint16_t m_readByteIndex;
	uint8_t m_inBuffer[5];

	uint64_t m_startReading;
	uint64_t m_endReading;

	uint32_t m_deltaReading;
	uint8_t m_isOnline;
	uint8_t m_errCode;

	uint16_t m_temperature;
	uint16_t m_humidity;

	uint32_t m_extiPin;

	Timer *m_pTimer;
	Timer *m_pTimeoutTimer;

	GPIO_InitTypeDef m_inputConfig;
	GPIO_InitTypeDef m_outputConfig;

	typedef enum  {
		DHTIdle,
		DHTExpectInitialResponse_Low,
		DHTExpectInitial_High,
		DHTExpectInitial_Low,
		DHTExpectData_Start_Data_High,
		DHTExpectData_End_Data_low,
		DHTCompleted
	} DHT_Read_States_e;

	DHT_Read_States_e m_state;

	void finalizeReading();
	void disableIRQ();
	void enableIRQ();
	void donePause();
	void timeout();

public:
	DHT22(uint8_t id, GPIO_TypeDef* port, uint16_t pin, uint32_t exti, uint32_t extiPinSource);
	void HandleTransition(uint32_t pr, uint32_t msk);
	void Update();
	float GetTemp();
	float GetHumidity();

	~DHT22();

	DHT22 *pNext;
};

extern "C" void ReadAllDHT22();
extern "C" void DH22HandleTransition(uint32_t pr, uint32_t msk);

