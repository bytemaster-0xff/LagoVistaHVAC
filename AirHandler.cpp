#include "AirHandler.h"
#include <string.h>
#include "ElectricImp.h"
AirHandler::AirHandler()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = RV_PIN;
	GPIO_Init(RV_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(RV_PORT, RV_PIN, Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = FAN_PIN;
	GPIO_Init(FAN_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(FAN_PORT, FAN_PIN, Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = COOL1_PIN;
	GPIO_Init(COOL1_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(COOL1_PORT, COOL1_PIN, Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = COOL2_PIN;
	GPIO_Init(COOL2_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(COOL2_PORT, COOL2_PIN, Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = HEAT1_PIN;
	GPIO_Init(HEAT1_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(HEAT1_PORT, HEAT1_PIN, Bit_RESET);

	GPIO_InitStructure.GPIO_Pin = HEAT2_PIN;
	GPIO_Init(HEAT2_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(HEAT2_PORT, HEAT2_PIN, Bit_RESET);
}


void AirHandler::Handle(Command *cmd){
	BitAction action;

	char *device = cmd->Args[0];
	char *state = cmd->Args[1];

	if (strcmp(state, "on") == 0)
		action = Bit_SET;
	else if (strcmp(state, "off") == 0)
		action = Bit_RESET;
	else
		return; /* Don't send ACK, invalid new state */

	if (strcmp(device, "rv") == 0)
		GPIO_WriteBit(RV_PORT, RV_PIN, action);
	else if (strcmp(device, "fan") == 0)
		GPIO_WriteBit(FAN_PORT, FAN_PIN, action);
	else if (strcmp(device, "cool1") == 0)
		GPIO_WriteBit(COOL1_PORT, COOL1_PIN, action);
	else if (strcmp(device, "cool2") == 0)
		GPIO_WriteBit(COOL2_PORT, COOL2_PIN, action);
	else if (strcmp(device, "heat1") == 0)
		GPIO_WriteBit(HEAT1_PORT, HEAT1_PIN, action);
	else if (strcmp(device, "heat2") == 0)
		GPIO_WriteBit(HEAT2_PORT, HEAT2_PIN, action);
	else
		return;

	Imp->Write(device);
	Imp->Write(",");
	Imp->Write(state);
	Imp->Write(",ok;");
}

bool AirHandler::GetState(char *) {

}

AirHandler::~AirHandler()
{
}
