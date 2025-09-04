#pragma once
#include <open62541/types.h>
#include "pidCalculate.h"

typedef struct {
    double pv;           /* ��������� PV (�C) */
    UA_StatusCode st;     /* ������ ���������� ������ */
} CashSensor;

typedef struct {
    PIDControllerData pid;
    CashSensor cash;
}ControlLoop;

/* �������������� ��� */
static void cash_init(CashSensor* t) {
    t->pv = 0.0;
    t->st = UA_STATUSCODE_BADDATAUNAVAILABLE;
}

/* ����������� ����� ������� � ��� */
void daq_tick(CashSensor* t);

void tick100ms_cb(UA_Server* server, void* ctx);