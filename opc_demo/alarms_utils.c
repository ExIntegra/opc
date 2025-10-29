#include "alarms_utils.h"

// Записать ActiveState/Id по текущим флагам состояния в loop->sensor.state
void update_active_flag(UA_Server* server, ControlLoop* loop) {
	/* Берём готовые флаги из твоей структуры состояния (после гистерезиса это правильнее) */
	UA_Boolean any = UA_FALSE;
	if (loop->sensor.state.highHigh)    any = UA_TRUE;
	else if (loop->sensor.state.high)   any = UA_TRUE;
	else if (loop->sensor.state.low)    any = UA_TRUE;
	else if (loop->sensor.state.lowLow) any = UA_TRUE;

    set_two_state_id(server, loop->sensor.alarmConditionId, "ActiveState", any);
}


// Устанавливаем уровень серьезности тревоги в зависимости от активной тревоги.
void set_severity(UA_Server* server, ControlLoop* loop) {

	// По умолчанию - нормальное состояние.
    UA_UInt16 severity = loop->sensor.limits.priorityNormal;

	// Проверяем, какая тревога активна и выставляем соответствующий уровень серьезности.
    if (loop->sensor.state.highHigh)    severity = loop->sensor.limits.priorityHighHighAlarm;
    else if (loop->sensor.state.lowLow) severity = loop->sensor.limits.priorityLowLowAlarm;
    else if (loop->sensor.state.high)   severity = loop->sensor.limits.priorityHighAlarm;
    else if (loop->sensor.state.low)    severity = loop->sensor.limits.priorityLowAlarm;

	// Ограничение сверху и снизу.
    if (severity > 1000) severity = 1000;
	if (severity <= 1)    severity = 1;

    UA_Variant v;
    UA_StatusCode vx = UA_Variant_setScalarCopy(&v, &severity, &UA_TYPES[UA_TYPES_UINT16]);
    if (vx == UA_STATUSCODE_GOOD) {
        UA_StatusCode rc = UA_Server_setConditionField(server, loop->sensor.alarmConditionId,
            &v, UA_QUALIFIEDNAME(0, "Severity"));
        if (rc != UA_STATUSCODE_GOOD) printf("Severity write failed: %s\n", UA_StatusCode_name(rc));
    }   
    UA_Variant_clear(&v);
}

// Устанавливаем сообщение тревоги в зависимости от активной тревоги.
void set_message_alarm(UA_Server* server, ControlLoop* loop) {
    char message[128];
    const AlarmLimits* lim = &loop->sensor.limits;
    const UA_Double pv = loop->sensor.io.pv;

    /* UA_String печатаем как %.*s */
    const int nameLen = (int)loop->sensor.name.length;
    const char* namePtr = (const char*)loop->sensor.name.data;

    if (loop->sensor.state.highHigh) {
        snprintf(message, sizeof(message),
            "ALARM HH: Sensor:%.*s PV=%.1f >= HH=%.1f",
            nameLen, namePtr, pv, lim->highHigh);
    }
    else if (loop->sensor.state.lowLow) {
        snprintf(message, sizeof(message),
            "ALARM LL: Sensor:%.*s PV=%.1f <= LL=%.1f",
            nameLen, namePtr, pv, lim->lowLow);
    }
    else if (loop->sensor.state.high) {
        snprintf(message, sizeof(message),
            "ALARM H:  Sensor:%.*s PV=%.1f >= H=%.1f",
            nameLen, namePtr, pv, lim->high);
    }
    else if (loop->sensor.state.low) {
        snprintf(message, sizeof(message),
            "ALARM L:  Sensor:%.*s PV=%.1f <= L=%.1f",
            nameLen, namePtr, pv, lim->low);
    }
    else {
        snprintf(message, sizeof(message),
            "Normal:   Sensor:%.*s PV=%.1f",
            nameLen, namePtr, pv);
    }

    UA_LocalizedText lt = UA_LOCALIZEDTEXT("en-US", message);
    UA_Variant v;
    if (UA_Variant_setScalarCopy(&v, &lt, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]) == UA_STATUSCODE_GOOD) {
        UA_StatusCode rcMsg = UA_Server_setConditionField(
            server, loop->sensor.alarmConditionId, &v, UA_QUALIFIEDNAME(0, "Message"));
        if (rcMsg != UA_STATUSCODE_GOOD) printf("Message write failed: %s\n", UA_StatusCode_name(rcMsg));
    }
    UA_Variant_clear(&v);

}

// Обновляем состояние тревог с гистерезисом.
UA_Boolean update_alarm_state_with_hyst(UA_Double pv, const AlarmLimits* lim, AlarmState* st)
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

// Обеспечиваем защиту по блокировкам HighHigh и LowLow.
void emergency_protection(ControlLoop* loop, UA_Double* ctrl) {
    if (loop->sensor.state.highHigh) {
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

// Вспомогательная функция для установки состояний тревоги в Condition.
void set_two_state_id(UA_Server* server, const UA_NodeId cond,
    const char* varName, UA_Boolean state) {
    UA_Variant v;
    UA_StatusCode vx = UA_Variant_setScalarCopy(&v, &state, &UA_TYPES[UA_TYPES_BOOLEAN]);
    if (vx != UA_STATUSCODE_GOOD) {
        printf("Variant copy failed in set_two_state_id(%s): %s\n", varName, UA_StatusCode_name(vx));
        return;
    }
    UA_StatusCode rc = UA_Server_setConditionVariableFieldProperty(
        server, cond, &v,
        UA_QUALIFIEDNAME(0, (char*)varName),
        UA_QUALIFIEDNAME(0, "Id"));
    if (rc != UA_STATUSCODE_GOOD) {
        printf("setTwoState(%s) failed: %s\n", varName, UA_StatusCode_name(rc));
    }
    UA_Variant_clear(&v);
}

