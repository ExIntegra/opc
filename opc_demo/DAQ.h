#pragma once
#include <open62541/server.h>
#include <open62541/types.h>

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

/* --- ��������� ������� --- */
typedef struct {
	UA_String name;
    UA_NodeId objId;
    UA_Boolean clampEnable;

    UA_Double outMin;
    UA_Double outMax;

    UA_Double command;
	UA_Double actual;

    UA_UInt32 actionHH;
    UA_UInt32 actionLL;
    UA_Double safeOutputHH;
    UA_Double safeOutputLL;
} Valve;

/* --- ��� ��� �������� �������� ������� � ������� ��������� --- */
typedef struct {
    double pv;
    UA_StatusCode st;     /* ������ ���������� ������ */
} CashSensor;

/* --- ��������� ������� --- */
typedef struct {
	UA_String name;
    CashSensor io;
    AlarmLimits limits;
    AlarmState state;
    UA_NodeId objId;            // NodeId ������� ������� � �������� ������������
    UA_NodeId alarmConditionId; // NodeId ConditionType ��� ������������
} Sensor;

/* --- ��������� PID ����������� --- */
typedef struct {
    UA_String name;
    UA_Double kp;
    UA_Double ki;
    UA_Double kd;
    UA_Double output;
    UA_Double manualoutput;
    UA_Double setpoint;
    UA_Double processvalue;
    UA_Double integral;
    UA_Double lastError;
    UA_Boolean mode;
} PIDControllerType;

/* --- ������ ������ ���������� � PID ������������, ����� � ��������� ������������ --- */
typedef struct {
	UA_String name;
    PIDControllerType pid;
	Sensor sensor;
    Valve valve;
} ControlLoop;

/* --- �������������� ��� --- */
static void cash_init(CashSensor* t) {
    t->pv = 0.0;
    t->st = UA_STATUSCODE_BADDATAUNAVAILABLE;
}

/* --- ������������� ������� --- */
static void valve_init(Valve* v) {
    v->name = UA_STRING_NULL;
    v->objId = UA_NODEID_NULL;
    v->clampEnable = UA_FALSE;
    v->outMin = 0.0;
    v->outMax = 100.0;
    v->command = 0.0;
    v->actual = 0.0;
    v->actionHH = PVFAIL_HOLD;
    v->actionLL = PVFAIL_HOLD;
    v->safeOutputHH = 0.0;
    v->safeOutputLL = 0.0;
}


/* --- ������������� ������� --- */
static void sensor_init(Sensor* s) {
    s->name = UA_STRING_NULL;
    cash_init(&s->io);
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
}

/* --- ������������� PID ����������� --- */
static inline void pid_init(PIDControllerType* pid)
{
    pid->name = UA_STRING_NULL;
    pid->kp = 0.0;
    pid->ki = 0.0;
    pid->kd = 0.0;
    pid->processvalue = 0.0;
    pid->output = 0.0;
    pid->integral = 0.0;
    pid->lastError = 0.0;
    pid->setpoint = 0.0;
    pid->manualoutput = 0.0;
    pid->mode = UA_FALSE;
}

// ����������� ����� ������� � ���.
void daq_tick(CashSensor* t);

// ������������ ����� ������� � �������� pid.
void tick(UA_Server* server, void* ctx);

// �������� ���������� ������� PID-���������� ����� � ��������������
void pidCalculate(PIDControllerType* pid);