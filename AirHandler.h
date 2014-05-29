#pragma once

#include <stm32f4xx_gpio.h>
#include "Command.h"

#define FAN_PORT GPIOE
#define FAN_PIN GPIO_Pin_6

#define RV_PORT GPIOC
#define RV_PIN GPIO_Pin_3

#define COOL1_PORT GPIOE
#define COOL1_PIN GPIO_Pin_2

#define COOL2_PORT GPIOE
#define COOL2_PIN GPIO_Pin_3

#define HEAT1_PORT GPIOE
#define HEAT1_PIN GPIO_Pin_4

#define HEAT2_PORT GPIOE
#define HEAT2_PIN GPIO_Pin_5

class AirHandler
{
public:
	AirHandler();
	void Handle(Command *cmd);
	bool GetState(char *);
	~AirHandler();
};

