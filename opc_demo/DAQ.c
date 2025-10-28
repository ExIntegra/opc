#include "DAQ.h"
#include "sensorsRead.h"
#include <math.h>

/* clamp */
static inline UA_Double clampd(UA_Double x, UA_Double lo, UA_Double hi) {
    return (x < lo) ? lo : (x > hi) ? hi : x;
}

/* отправка команды на исполнитель — пустышка (команда уже в valve.command) */
static void actuator_send(UA_Double command) { (void)command; }

/* чтение фидбэка — если нет датчика, вернём NaN, tick подставит actual=command */
static UA_Double actuator_readback(void) { return NAN; }

// Обновление данных с датчика температуры DS18B20

void daq_tick(CashSensor* sensor) {

	static UA_Boolean inited = UA_FALSE;

	if (!inited) {
		sensor->pv = 50.0;              // стартовое значение один раз
		inited = UA_TRUE;
	}
	else {
		sensor->pv += 1.0;              // +1 на каждый тик
		// при желании — зацикливание:
		// if(sensor->pv > 100.0) sensor->pv = 0.0;
	}

	sensor->st = UA_STATUSCODE_GOOD;    // статус OK

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
		// Обновляем состояние тревог с гистерезисом. Если состояние изменилось, триггерим событие и отрабатываем Condition, уведомляя клиента.
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

			// Обновляем флаг активности тревоги в Condition.
			updateActiveFlag(server, loop);
			// Устанавливаем уровень серьезности тревоги (Severity). Необходим для фильтрации на клиенте (цвет, звук, и т.д.)
            setSeverity(server, loop);
			// Устанавливаем сообщение тревоги,уведомляем пользователя (сигнализация на HMI)
			setMessageAlarm(server, loop);

			// Обновляем текущее значение PV в Condition для отображения в клиенте
            (void)UA_Server_setLimitState(server, loop->sensor.alarmConditionId, pv);

			// Триггерим событие изменения состояния тревоги
            (void)UA_Server_triggerConditionEvent(server,
                loop->sensor.alarmConditionId,  /* кто генерит: сама Condition (LimitAlarm) */
                loop->sensor.objId,             /* источник: объект Sensor */
				NULL);                          /* дополнительные данные не нужны */
        }
    }

	// Реагируем на срабатывание блокировки, если таковая имеется
	emergency_protection(loop, &ctrl);

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