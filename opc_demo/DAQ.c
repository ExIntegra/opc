//� ���� ������ ������������ ������� � ����������� �������

#include "DAQ.h"
#include "sensorsRead.h"
#include <stdio.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>


// ����������� ����� ������� � ���
void daq_tick(CashSensor* sensor) {
    double value;
    UA_StatusCode rc = ds18b20_readC(&value);
    if (rc == UA_STATUSCODE_GOOD) {
        sensor->pv = value;
        sensor->st = UA_STATUSCODE_GOOD;
    }
    else {
        sensor->st = rc;      /* pv �� ������� � ������� ���������� */
    }
}

/* ������: ��� � 100�� ���������� ������ � ������������� pid1 */
void tick100ms_cb(UA_Server* server, void* ctx) {
    ControlLoop* cashLoop = (ControlLoop*)ctx;
    (void)server;
    UA_DateTimeStruct t = UA_DateTime_toStruct(UA_DateTime_now());
    printf("%02u:%02u:%02u.%03u\n",
        (unsigned)t.hour, (unsigned)t.min, (unsigned)t.sec, (unsigned)t.milliSec);
    daq_tick(&cashLoop->cash);

    /* 2) ���� ������ �������� � ��������� PV ��� pid1 � ������� ����� */
    if (cashLoop->cash.st == UA_STATUSCODE_GOOD) {
        cashLoop->pid.processvalue = cashLoop->cash.pv; /* ����� pid1 <- DAQ */
        pidCalculate(&cashLoop->pid);              /* ���� ������� ������� */
        //actuatorApply(cashLoop->pid.output);
    }
}
