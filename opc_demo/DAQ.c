#include "DAQ.h"
#include "sensorsRead.h"
#include <math.h>

/* clamp */
static inline UA_Double clampd(UA_Double x, UA_Double lo, UA_Double hi) {
    return (x < lo) ? lo : (x > hi) ? hi : x;
}

/* гистерезис тревог */
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

/* отправка команды на исполнитель — пустышка (команда уже в valve.command) */
static void actuator_send(UA_Double command) { (void)command; }

/* чтение фидбэка — если нет датчика, вернём NaN, tick подставит actual=command */
static UA_Double actuator_readback(void) { return NAN; }

// Обновление данных с датчика температуры DS18B20
void daq_tick(CashSensor* sensor) {

    sensor->pv = 50.0; //для отладки
	sensor->st = UA_STATUSCODE_GOOD; //для отладки


	//Реальное чтение с датчика DS18B20 закомментировано для отладки


    /*/////////////////////////////////////////////////////////////////
    UA_Double value;
    UA_StatusCode rc = ds18b20_readC(&value);
    if (rc == UA_STATUSCODE_GOOD) {
		sensor->pv = value; // Обновляем значение PV
		sensor->st = UA_STATUSCODE_GOOD; // Обновляем статус на GOOD
    }
    else {
		sensor->st = rc; // Обновляем статус ошибки
    }
    */////////////////////////////////////////////////////////////////
}

/* Колбэк: раз в 100мс опрашиваем датчик.
   В завимости от статуса опроса датчика, политики безопаснои клапана и его режима строится поведение системы */
void tick(UA_Server* server, void* ctx)
{
    if (!server || !ctx) return;

    ControlLoop* loop = (ControlLoop*)ctx;

    /* Опрашиваем датчик в кэш */
    daq_tick(&loop->sensor.io);

	/* 1) Вычисляем команду клапану в завимости от режима PID и статуса PV */ 
    const UA_Double     pv = loop->sensor.io.pv;
    const UA_StatusCode pvSt = loop->sensor.io.st;
    const UA_Boolean    pvGood = UA_StatusCode_isGood(pvSt);

    /* Базовая команда: AUTO (PID) при хорошем PV, иначе MAN: */

    UA_Double ctrl; //конечная команда на исполняющее устройство
    
    //если статус чтения ликвиден и режим работы в AUTO (TRUE)
    if (pvGood && loop->pid.mode) {
        loop->pid.processvalue = pv;  //записываем в структуру pid pv
        pidCalculate(&loop->pid);     //расчет управляющего сигнала,
                                      //результат кладем в pid.output
        ctrl = loop->pid.output;      //присваием конечной команде ctrl выходной сигнал после расчета pidCalculate
    }
    // если статус чтения с ошибкой или режим работы MAN (FALSE),
    // то используем в качесве степени открытия не ПИД-расчет, а значение,
    // которое задал оператор через HMI.
    else {
        ctrl = loop->pid.manualoutput;
    }

    /* Обновляем тревоги с гистерезисом и шлём Condition-событие при изменении */
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

    /* Реакция на блокировки (приоритет HH > LL) в зависимости от политика безопасности */

    // Проверяем флаг срабатывания блокировки.
    if (loop->sensor.state.highHigh) {
        // В случае срабатывания блокировки, действуем согласно прописанной политики безопасности.
        switch ((PvFailAction)loop->valve.actionHH) {
        case PVFAIL_HOLD:    /* оставить ctrl как есть */ break;
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

    /* Ограничение команд физическими пределами клапана */
    if (loop->valve.clampEnable) {
        ctrl = clampd(ctrl, loop->valve.outMin, loop->valve.outMax);
    }

    /* Отправка команды и фактический отклик */
    loop->valve.command = ctrl;
    actuator_send(loop->valve.command);

    UA_Double fb = actuator_readback();
    loop->valve.actual = (fb == fb) ? fb : loop->valve.command;  /* NaN-проверка: fb==fb */
}