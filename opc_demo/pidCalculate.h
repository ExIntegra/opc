#pragma once
#include "DAQ.h"
#include "config.h"
#include "types.h"
#include <open62541/types.h>
void pidCalculate(PIDControllerType* pid, UA_Double outMin, UA_Double outMax);