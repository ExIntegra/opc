#pragma once
#include <open62541/types.h>

void actuator_valve_send(UA_Double command);
UA_Double clampd(UA_Double x, UA_Double lo, UA_Double hi);
