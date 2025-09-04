#pragma once
#include <open62541/types.h>
#include "pidCalculate.h"

typedef struct {
    double pv;           /* последнее PV (°C) */
    UA_StatusCode st;     /* статус последнего чтения */
} CashSensor;

typedef struct {
    PIDControllerData pid;
    CashSensor cash;
}ControlLoop;

/* инициализируем кэш */
static void cash_init(CashSensor* t) {
    t->pv = 0.0;
    t->st = UA_STATUSCODE_BADDATAUNAVAILABLE;
}

/* однократный опрос датчика в кэш */
void daq_tick(CashSensor* t);

void tick100ms_cb(UA_Server* server, void* ctx);