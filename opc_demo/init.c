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
    v->clampEnable = UA_TRUE;
    v->outMin = 0.0;
    v->outMax = 100.0;
    v->command = 0.0;
    v->actionHH = PVFAIL_HOLD;
    v->actionLL = PVFAIL_HOLD;
    v->safeOutputHH = 0.0;
    v->safeOutputLL = 0.0;
    v->actual_position = v->command;
}

void reactor_init(Reactor* r) {
    r->name = UA_STRING_NULL;
    r->objId = UA_NODEID_NULL;
    r->mixer = UA_TRUE;
    r->volume = 100;
}

void valve_handle_control_init(ValveHandleControl* vhc) {
    vhc->name = UA_STRING_NULL;
    vhc->objId = UA_NODEID_NULL;
    vhc->manualoutput = 0.0;
}

// Инициализация датчика
void sensor_init(Sensor* s) {
    cash_init(&s->io);
    s->name = UA_STRING_NULL;
    s->limits.low = 0.0;
    s->limits.lowLow = 0.0;
    s->limits.high = 0.0;
    s->limits.highHigh = 0.0;
    s->limits.hysteresis = 0.0;
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

void model_init(ModelCtx* m, Sensor* sensorTemperature, Sensor* sensorF, Sensor* sensorConcentrationA,
    Sensor* sensorConcentrationB, Reactor* reactor, ValveHandleControl* valveRegulationConcentrationA,
    ValveHandleControl* valveRegulationQ, ValveHandleControl* valveRegulationT) {
    m->reactor = reactor;
    m->valveRegulationQ = valveRegulationQ;
	m->valveRegulationConcentrationA = valveRegulationConcentrationA;
    m->valveRegulationT = valveRegulationT;
    //m->sensorTemperature = sensorTemperature;
    m->sensorF = sensorF;
    m->sensorConcentrationA = sensorConcentrationA;
    m->sensorConcentrationB = sensorConcentrationB;
    m->sensorT = sensorTemperature;
    m->cfg.R = 8.314;
    m->cfg.k01 = 0;
    m->cfg.k02 = 0;
    m->cfg.EA1 = 0;
    m->cfg.EA2 = 0;
    m->substanceId = 0;
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
    pid->setpoint = 20.0;
    pid->manualoutput = 0.0;
    pid->mode = UA_FALSE;
}