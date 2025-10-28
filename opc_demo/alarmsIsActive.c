#include <open62541/server.h>
#include <open62541/types.h>
#include "DAQ.h"
#include <stdio.h>


/* Записать ActiveState/Id по текущим флагам состояния в loop->sensor.state */
void updateActiveFlag(UA_Server* server, ControlLoop* loop) {
	/* Берём готовые флаги из твоей структуры состояния (после гистерезиса это правильнее) */
	UA_Boolean any = UA_FALSE;
	if (loop->sensor.state.highHigh) any = UA_TRUE;
	else if (loop->sensor.state.high) any = UA_TRUE;
	else if (loop->sensor.state.low) any = UA_TRUE;
	else if (loop->sensor.state.lowLow) any = UA_TRUE;

	/* Пишем в Condition: ActiveState/Id */
	UA_StatusCode rc = UA_Server_writeObjectProperty_scalar(server,
		loop->sensor.alarmConditionId,                 /* узел твоей тревоги (LimitAlarm) */
		UA_QUALIFIEDNAME(0, "ActiveState/Id"),
		&any,
		&UA_TYPES[UA_TYPES_BOOLEAN]
	);

	if (rc != UA_STATUSCODE_GOOD) {
		printf("ActiveState write failed: %s\n", UA_StatusCode_name(rc));
	}
}


// Устанавливаем уровень серьезности тревоги в зависимости от активной тревоги.
void setSeverity(UA_Server* server, ControlLoop* loop) {
    UA_UInt16 severity = loop->sensor.limits.priorityNormal;  /* по умолчанию */

    if (loop->sensor.state.highHigh) {
        severity = loop->sensor.limits.priorityHighHighAlarm;
    }
    else if (loop->sensor.state.lowLow) {
        severity = loop->sensor.limits.priorityLowLowAlarm;
    }
    else if (loop->sensor.state.high) {
        severity = loop->sensor.limits.priorityHighAlarm;
    }
    else if (loop->sensor.state.low) {
        severity = loop->sensor.limits.priorityLowAlarm;
    }

    /* на всякий случай зажмём диапазон 0..1000 */
    if (severity > 1000) severity = 1000;

    UA_Variant v;
    UA_Variant_setScalar(&v, &severity, &UA_TYPES[UA_TYPES_UINT16]);
	//!!! Узнать про UA_Server_setConditionField !!!
    UA_StatusCode rc = UA_Server_setConditionField(
        server,
        loop->sensor.alarmConditionId,         /* твой узел LimitAlarm */
        &v,
        UA_QUALIFIEDNAME(0, "Severity")
    );
    if (rc != UA_STATUSCODE_GOOD) {
        printf("Severity write failed: %s\n", UA_StatusCode_name(rc));
    }
}

// Устанавливаем сообщение тревоги в зависимости от активной тревоги.
void setMessageAlarm(UA_Server* server, ControlLoop* loop) {

    char message[128];
    const AlarmLimits* lim = &loop->sensor.limits;
	const UA_Double pv = loop->sensor.io.pv;    
    const int nameLen = (int)loop->sensor.name.length;
    const char* namePtr = (const char*)loop->sensor.name.data;

    if (loop->sensor.state.highHigh) {
        snprintf(message, sizeof(message), "ALARM HH: Sensor:%.*s HighHigh(%.1f)", nameLen, namePtr, pv);
    }
    else if (loop->sensor.state.lowLow) {
        snprintf(message, sizeof(message), "ALARM LL: Sensor:%.*s LowLow(%.1f)", nameLen, namePtr, pv);
    }
    else if (loop->sensor.state.high) {
        snprintf(message, sizeof(message), "ALARM H: Sensor:%.*s High(%.1f)", nameLen, namePtr, pv);
    }
    else if (loop->sensor.state.low) {
        snprintf(message, sizeof(message), "ALARM L: Sensor:%.*s Low(%.1f)", nameLen, namePtr, pv);
    }
    else {
        snprintf(message, sizeof(message), "Normal: Sensor:%.*s PV=%.1f", nameLen, namePtr, pv);
    }

    UA_LocalizedText text = UA_LOCALIZEDTEXT("en-US", message);
    UA_Variant v;
    UA_Variant_setScalar(&v, &text, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    //!!! Узнать про UA_Server_setConditionField !!!
    UA_StatusCode rc = UA_Server_setConditionField(server,loop->sensor.alarmConditionId,
        &v, UA_QUALIFIEDNAME(0, "Message"));

    if (rc != UA_STATUSCODE_GOOD) {
        printf("Message write failed: %s\n", UA_StatusCode_name(rc));
    }
}

// Обновляем состояние тревог с гистерезисом.
UA_Boolean updateAlarmStateWithHyst(UA_Double pv, const AlarmLimits* lim, AlarmState* st)
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

void emergency_protection(ControlLoop* loop, UA_Double* ctrl) {
    if (loop->sensor.state.highHigh) {
        // В случае срабатывания блокировки, действуем согласно прописанной политики безопасности.
        switch ((PvFailAction)loop->valve.actionHH) {
        case PVFAIL_HOLD:    /* оставить ctrl как есть */ break;
        case PVFAIL_TO_MAN:  loop->pid.mode = UA_FALSE; *ctrl = loop->pid.manualoutput; break;
        case PVFAIL_TO_SAFE: *ctrl = loop->valve.safeOutputHH; break;
        }
    }
    else if (loop->sensor.state.lowLow) {
        switch ((PvFailAction)loop->valve.actionLL) {
        case PVFAIL_HOLD:    break;
        case PVFAIL_TO_MAN:  loop->pid.mode = UA_FALSE; *ctrl = loop->pid.manualoutput; break;
        case PVFAIL_TO_SAFE: *ctrl = loop->valve.safeOutputLL; break;
        }
    }
}