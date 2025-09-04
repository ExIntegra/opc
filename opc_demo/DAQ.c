//В этом модуле опрашиваются датчики и формируются события

#include "DAQ.h"
#include "sensorsRead.h"
#include <stdio.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>


// однократный опрос датчика в кэш
void daq_tick(CashSensor* sensor) {
    double value;
    UA_StatusCode rc = ds18b20_readC(&value);
    if (rc == UA_STATUSCODE_GOOD) {
        sensor->pv = value;
        sensor->st = UA_STATUSCODE_GOOD;
    }
    else {
        sensor->st = rc;      /* pv не трогаем — остаётся предыдущее */
    }
}

/* Колбэк: раз в 100мс опрашиваем датчик и пересчитываем pid1 */
void tick100ms_cb(UA_Server* server, void* ctx) {
    ControlLoop* cashLoop = (ControlLoop*)ctx;
    (void)server;
    UA_DateTimeStruct t = UA_DateTime_toStruct(UA_DateTime_now());
    printf("%02u:%02u:%02u.%03u\n",
        (unsigned)t.hour, (unsigned)t.min, (unsigned)t.sec, (unsigned)t.milliSec);
    daq_tick(&cashLoop->cash);

    /* 2) если чтение успешное — обновляем PV для pid1 и считаем выход */
    if (cashLoop->cash.st == UA_STATUSCODE_GOOD) {
        cashLoop->pid.processvalue = cashLoop->cash.pv; /* связь pid1 <- DAQ */
        pidCalculate(&cashLoop->pid);              /* твоя функция расчёта */
        //actuatorApply(cashLoop->pid.output);
    }
}
