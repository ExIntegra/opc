#include "init.h"

// Инициализация кассового датчика
void cash_init(CashSensor* t) {
    t->pv = 0.0;
    t->st = UA_STATUSCODE_BADDATAUNAVAILABLE;
}

// Инициализация клапана
void valve_init(Valve* v) {
    v->name = UA_STRING_NULL;
    v->objId = UA_NODEID_NULL;
    v->clampEnable = UA_FALSE;
    v->outMin = 0.0;
    v->outMax = 100.0;
    v->command = 0.0;
    v->actionHH = PVFAIL_HOLD;
    v->actionLL = PVFAIL_HOLD;
    v->safeOutputHH = 0.0;
    v->safeOutputLL = 0.0;
    v->actual_position = v->command;
}

// Инициализация датчика
void sensor_init(Sensor* s) {
    cash_init(&s->io);
    s->name = UA_STRING_NULL;
    s->limits.low = 0.0;
    s->limits.lowLow = 0.0;
    s->limits.high = 100.0;
    s->limits.highHigh = 100.0;
    s->limits.hysteresis = 1.0;
    s->state.low = UA_FALSE;
    s->state.lowLow = UA_FALSE;
    s->state.high = UA_FALSE;
    s->state.highHigh = UA_FALSE;
    s->objId = UA_NODEID_NULL;
    s->alarmConditionId = UA_NODEID_NULL;
    s->limits.priorityLowAlarm = 500;
    s->limits.priorityHighAlarm = 500;
    s->limits.priorityLowLowAlarm = 1000;
    s->limits.priorityHighHighAlarm = 1000;
    s->limits.priorityNormal = 0;
}

// Инициализация ПИД-регулятора
void pid_init(PIDControllerType* pid) {
    pid->name = UA_STRING_NULL;
    pid->kp = 1.0;
    pid->ki = 1.0;
    pid->kd = 0.0;
    pid->processvalue = 0.0;
    pid->output = 0.0;
    pid->integral = 0.0;
    pid->lastError = 0.0;
    pid->setpoint = 70.0;
    pid->manualoutput = 0.0;
    pid->mode = UA_TRUE;
}