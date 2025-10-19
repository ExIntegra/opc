#pragma once
#include <open62541/server.h>
#include <open62541/types.h>
#include "pidCalculate.h"

/* --- ����� �������� ������������ ������� ---*/
typedef enum {
    PVFAIL_HOLD = 0,   /* ������� ������� output */
    PVFAIL_TO_MAN,     /* ����� ��������� ������ � MAN � ����� manualoutput */
    PVFAIL_TO_SAFE     /* ��������� ������������� ���������� ����� safeOutput */
} PvFailAction;

/* --- �������� ������������ --- */
typedef struct {
    PvFailAction action;        // ��������� ���������
    UA_Boolean clampEnable;   // �������� ����������� ������
    UA_Double outMin;        // ������ ������� ������ (������� �������� �������)
    UA_Double outMax;        // ������� ������� ������ (������� �������� �������)
    UA_Double safeOutput;    // ������� �������� ������� � ���������� ���������
} SafetyPolicy;

/* --- ��������� �������� ������������ � ���������� --- */
typedef struct {
    UA_Double low;
    UA_Double lowLow;
    UA_Double high;
    UA_Double highHigh;
    UA_Double hysteresis; // ���������� ��� ��������. �������� �� ��������.
} AlarmLimits;

/* --- ��������� �������� ������������ --- */
typedef struct {
    UA_Boolean lowLow;
    UA_Boolean low;
    UA_Boolean high;
    UA_Boolean highHigh;
} AlarmState;

typedef struct {
	UA_String name;
    UA_NodeId objId;
    UA_Boolean clampEnable;

    UA_Double outMin;
    UA_Double outMax;

    UA_UInt32 actionHH;
    UA_UInt32 actionLL;
    UA_Double safeOutputHH;
    UA_Double safeOutputLL;

    UA_Double command;
	UA_Double actual;
} Valve;


/* --- ��� ��� �������� �������� ������� � ������� ��������� --- */
typedef struct {
    double pv;
    UA_StatusCode st;     /* ������ ���������� ������ */
} CashSensor;

typedef struct {
	UA_String name;
    CashSensor io;
    AlarmLimits limits;
    AlarmState state;
    UA_NodeId objId;            // NodeId ������� ������� � �������� ������������
    UA_NodeId alarmConditionId; // NodeId ConditionType ��� ������������
} Sensor;

/* --- ������ ������ ���������� � PID ������������, ����� � ��������� ������������ --- */
typedef struct {
	UA_String name;
    PIDControllerData pid;
	Sensor sensor;
    Valve valve;
} ControlLoop;


/* --- �������������� ��� --- */
static void cash_init(CashSensor* t) {
    t->pv = 0.0;
    t->st = UA_STATUSCODE_BADDATAUNAVAILABLE;
}

// ����������� ����� ������� � ���.
void daq_tick(CashSensor* t);

// ������������ ����� ������� � �������� pid.
void tick(UA_Server* server, void* ctx);