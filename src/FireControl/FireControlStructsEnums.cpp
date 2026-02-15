#include "FireControlStructsEnums.h"

const char* fireModeStr[3] = {"Single", "Burst", "Auto"};
const char* sensorStateStr[] = {"MID_CYCLE", "PRIMED", "CLOSED_BREACH", "FIRE_READY"};
const char* stateMachineStr[5] = {"LEAVING_STARTING_POSSITION", "CYCLE_TO_PRIMED", "CYCLE_TO_DEPRIMED", "COMPLETE_STATE", "ERROR_STATE"};
const char* idleModeStr[2] = {"DEPRIMED_IDLE", "PRIMED_IDLE"};
