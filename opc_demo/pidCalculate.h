#pragma once
#include <open62541/types.h>

typedef struct {
    UA_String name;
    UA_Double kp;
    UA_Double ki;
    UA_Double kd;
    UA_Double output;
    UA_Double setpoint;
    UA_Double processvalue;
    UA_Double integral;
    UA_Double lastError;
    UA_Boolean mode;
} PIDControllerData;


static inline void PID_init(PIDControllerData* pid)
{
    pid->kp = 0.0;
    pid->ki = 0.0;
    pid->kd = 0.0;
    pid->processvalue = 0.0;
    pid->output = 0.0;
    pid->integral = 0.0;
    pid->lastError = 0.0;
    pid->setpoint = 0.0;
}

void pidCalculate(PIDControllerData* pid);