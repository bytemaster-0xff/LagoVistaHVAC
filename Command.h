#pragma once
#include <stm32f4xx.h>

#define COMMAND_BUFFER_LENGTH 10
#define MAX_CMD_LENGTH 10
#define MAX_ARG_LENGTH 10
#define MAX_ARGS 10

struct Command
{
public:
	uint8_t NumberArgs;
	char Command[MAX_CMD_LENGTH];
	char Args[MAX_ARGS][MAX_ARG_LENGTH];
};

extern Command *CommandBuffer[];

extern uint8_t CommandBufferHead;
extern uint8_t CommandBufferTail;
