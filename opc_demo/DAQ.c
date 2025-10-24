#include "DAQ.h"
#include "sensorsRead.h"
#include <math.h>

/* clamp */
static inline UA_Double clampd(UA_Double x, UA_Double lo, UA_Double hi) {
    return (x < lo) ? lo : (x > hi) ? hi : x;
}

/* ���������� ������ */
static UA_Boolean updateAlarmStateWithHyst(UA_Double pv,
    const AlarmLimits* lim,
    AlarmState* st)
{
    UA_Boolean changed = UA_FALSE;
    const UA_Boolean oLL = st->lowLow, oL = st->low, oH = st->high, oHH = st->highHigh;
    const UA_Double h = lim->hysteresis;

    if (!st->highHigh) { if (pv >= lim->highHigh) st->highHigh = UA_TRUE; }
    else { if (pv <= lim->highHigh - h) st->highHigh = UA_FALSE; }

    if (!st->high) { if (pv >= lim->high) st->high = UA_TRUE; }
    else { if (pv <= lim->high - h) st->high = UA_FALSE; }

    if (!st->lowLow) { if (pv <= lim->lowLow) st->lowLow = UA_TRUE; }
    else { if (pv >= lim->lowLow + h) st->lowLow = UA_FALSE; }

    if (!st->low) { if (pv <= lim->low) st->low = UA_TRUE; }
    else { if (pv >= lim->low + h) st->low = UA_FALSE; }

    if (oLL != st->lowLow || oL != st->low || oH != st->high || oHH != st->highHigh)
        changed = UA_TRUE;
    return changed;
}

/* �������� ������� �� ����������� � �������� (������� ��� � valve.command) */
static void actuator_send(UA_Double command) { (void)command; }

/* ������ ������� � ���� ��� �������, ����� NaN, tick ��������� actual=command */
static UA_Double actuator_readback(void) { return NAN; }

// ���������� ������ � ������� ����������� DS18B20
void daq_tick(CashSensor* sensor) {

    sensor->pv = 50.0; //��� �������
	sensor->st = UA_STATUSCODE_GOOD; //��� �������


	//�������� ������ � ������� DS18B20 ���������������� ��� �������


    /*/////////////////////////////////////////////////////////////////
    UA_Double value;
    UA_StatusCode rc = ds18b20_readC(&value);
    if (rc == UA_STATUSCODE_GOOD) {
		sensor->pv = value; // ��������� �������� PV
		sensor->st = UA_STATUSCODE_GOOD; // ��������� ������ �� GOOD
    }
    else {
		sensor->st = rc; // ��������� ������ ������
    }
    */////////////////////////////////////////////////////////////////
}

/* ������: ��� � 100�� ���������� ������.
   � ��������� �� ������� ������ �������, �������� ���������� ������� � ��� ������ �������� ��������� ������� */
void tick(UA_Server* server, void* ctx)
{
    if (!server || !ctx) return;

    ControlLoop* loop = (ControlLoop*)ctx;

    /* ���������� ������ � ��� */
    daq_tick(&loop->sensor.io);

	/* 1) ��������� ������� ������� � ��������� �� ������ PID � ������� PV */ 
    const UA_Double     pv = loop->sensor.io.pv;
    const UA_StatusCode pvSt = loop->sensor.io.st;
    const UA_Boolean    pvGood = UA_StatusCode_isGood(pvSt);

    /* ������� �������: AUTO (PID) ��� ������� PV, ����� MAN: */

    UA_Double ctrl; //�������� ������� �� ����������� ����������
    
    //���� ������ ������ �������� � ����� ������ � AUTO (TRUE)
    if (pvGood && loop->pid.mode) {
        loop->pid.processvalue = pv;  //���������� � ��������� pid pv
        pidCalculate(&loop->pid);     //������ ������������ �������,
                                      //��������� ������ � pid.output
        ctrl = loop->pid.output;      //��������� �������� ������� ctrl �������� ������ ����� ������� pidCalculate
    }
    // ���� ������ ������ � ������� ��� ����� ������ MAN (FALSE),
    // �� ���������� � ������� ������� �������� �� ���-������, � ��������,
    // ������� ����� �������� ����� HMI.
    else {
        ctrl = loop->pid.manualoutput;
    }

    /* ��������� ������� � ������������ � ��� Condition-������� ��� ��������� */
    if (!UA_NodeId_isNull(&loop->sensor.alarmConditionId) && pvGood) {
        if (updateAlarmStateWithHyst(pv, &loop->sensor.limits, &loop->sensor.state)) {
            UA_Boolean T = UA_TRUE, F = UA_FALSE;

            (void)UA_Server_writeObjectProperty_scalar(server, loop->sensor.alarmConditionId,
                UA_QUALIFIEDNAME(0, "HighState/Id"),
                loop->sensor.state.high ? &T : &F, &UA_TYPES[UA_TYPES_BOOLEAN]);

            (void)UA_Server_writeObjectProperty_scalar(server, loop->sensor.alarmConditionId,
                UA_QUALIFIEDNAME(0, "HighHighState/Id"),
                loop->sensor.state.highHigh ? &T : &F, &UA_TYPES[UA_TYPES_BOOLEAN]);

            (void)UA_Server_writeObjectProperty_scalar(server, loop->sensor.alarmConditionId,
                UA_QUALIFIEDNAME(0, "LowState/Id"),
                loop->sensor.state.low ? &T : &F, &UA_TYPES[UA_TYPES_BOOLEAN]);

            (void)UA_Server_writeObjectProperty_scalar(server, loop->sensor.alarmConditionId,
                UA_QUALIFIEDNAME(0, "LowLowState/Id"),
                loop->sensor.state.lowLow ? &T : &F, &UA_TYPES[UA_TYPES_BOOLEAN]);

            (void)UA_Server_triggerConditionEvent(server,
                loop->sensor.alarmConditionId, loop->sensor.objId, NULL);
        }
    }

    /* ������� �� ���������� (��������� HH > LL) � ����������� �� �������� ������������ */

    // ��������� ���� ������������ ����������.
    if (loop->sensor.state.highHigh) {
        // � ������ ������������ ����������, ��������� �������� ����������� �������� ������������.
        switch ((PvFailAction)loop->valve.actionHH) {
        case PVFAIL_HOLD:    /* �������� ctrl ��� ���� */ break;
        case PVFAIL_TO_MAN:  loop->pid.mode = UA_FALSE; ctrl = loop->pid.manualoutput; break;
        case PVFAIL_TO_SAFE: ctrl = loop->valve.safeOutputHH; break;
        }
    }
    else if (loop->sensor.state.lowLow) {
        switch ((PvFailAction)loop->valve.actionLL) {
        case PVFAIL_HOLD:    break;
        case PVFAIL_TO_MAN:  loop->pid.mode = UA_FALSE; ctrl = loop->pid.manualoutput; break;
        case PVFAIL_TO_SAFE: ctrl = loop->valve.safeOutputLL; break;
        }
    }

    /* ����������� ������ ����������� ��������� ������� */
    if (loop->valve.clampEnable) {
        ctrl = clampd(ctrl, loop->valve.outMin, loop->valve.outMax);
    }

    /* �������� ������� � ����������� ������ */
    loop->valve.command = ctrl;
    actuator_send(loop->valve.command);

    UA_Double fb = actuator_readback();
    loop->valve.actual = (fb == fb) ? fb : loop->valve.command;  /* NaN-��������: fb==fb */
}