#pragma once
#include <open62541/types.h>
#include "types.h"

void cash_init(CashSensor* t);
void valve_init(Valve* v);
void sensor_init(Sensor* s);
void pid_init(PIDControllerType* pid);