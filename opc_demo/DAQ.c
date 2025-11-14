#include "DAQ.h"
#include "alarms_utils.h"
#include "actuator.h"
#include "sensorsRead.h"
#include "pidCalculate.h"
#include "types.h"

/*  Функция tick() предназначена для обновления  раз в N секунд обновлять, собирать данные с датчиков и выполнять управление
ПИД-регулятором. Также в случае достижения предельных значений будет срабатывать аварийная сигнализация и отработана поведение
клапана в зависимости от его политики безопасности.

Поэтапно происходит следующее:

1. daq_tick(&loop->sensor.io) опрашивает датчик и обновляет его состояние в кэше loop->sensor.io.pv и loop->sensor.io.st.
   - Для упрощения реализации в функции используется считывание одного фиктивного датчика температуры DS18B20.
     Регистрируется статус считывания (GOOD или ошибка).

2. Вычисляется управляющая команда ctrl для клапана:
   - Если статус считывания датчика хороший (GOOD) и режим работы ПИД-регулятора установлен в AUTO (TRUE),
     то выполняется расчет управляющего сигнала ПИД-регулятора на основе текущего значения PV с помощью функции pidCalculate().
   - Если статус считывания плохой (ошибка) или режим работы установлен в MAN (FALSE),
     то используется в качестве степени открытия не ПИД-расчет, а значение,
     которое задал оператор через HMI (человеко-машинный интерфейс).

3. Проверяется наличие зарегистрированной тревоги (Condition)* для датчика и статус считывания pvGood.
   - Если тревога есть и статус считывания хороший (GOOD),
     то вызывается функция updateAlarmStateWithHyst() для обновления состояния тревог с гистерезисом*.
	 Было реализовано 4 тревоги: HighHigh, High, Low, LowLow. С помощью функции updateAlarmStateWithHyst()
	 происходит сравнение текущего значения PV с порогами тревог и обновление состояний тревог в структуре loop->sensor.state.

4. Если состояние тревог изменилось, то обновляются соответствующие состояния в Condition.
   - Для этого вызывается функция set_two_state_id(). Эта функция устанавливает значения состояний тревог
    (HighState, HighHighState, LowState, LowLowState).
   - Данное действие позволяет клиентам OPC UA получать актуальную информацию о состоянии тревог.

5. Вызывается функция updateActiveFlag() для обновления флага активности тревоги в Condition.
   - Эта функция проверяет, активна ли любая из тревог (HighHigh, High, Low, LowLow) и
   устанавливает соответствующий флаг ActiveState в Condition.

6. Вызывается функция setSeverity() для установки уровня серьезности тревоги (Severity*) в Condition.
   - Уровень серьезности зависит от того, какая тревога активна.
   - Это позволяет клиентам OPC UA фильтровать тревоги по цвету, звуку и другим параметрам.

7. Вызывается функция setMessageAlarm() для установки сообщения тревоги в Condition.
   - Сообщение содержит информацию о типе тревоги и текущем значении PV.
   - Это уведомляет пользователя о возникшей тревоге через HMI.

8. Обновляется флаги тревог в Condition с помощью UA_Server_setLimitState().
   - Эта функция обновляет внутреннее состояние тревоги в соответствии с текущим значением PV.

9. Вызывается функция UA_Server_triggerConditionEvent() для генерации события тревоги.
   - Это уведомляет всех подписанных клиентов OPC UA о возникшей тревоге.

10. Вызывается функция emergency_protection() для выполнения аварийного отключения контура управления.
   - Это позволяет предотвратить повреждение оборудования и обеспечить безопасность.
   - Функция корректирует управляющий сигнал ctrl в зависимости от состояния тревог и политики безопасности клапана.

11. Если включено ограничение выходного сигнала клапана (clampEnable),
	то управляющий сигнал ctrl ограничивается в пределах outMin и outMax с помощью функции clampd().

12. Обновляются поля command и actual структуры клапана loop->valve.
   - command устанавливается в значение ctrl.
   - actual также устанавливается в значение ctrl (предполагается, что клапан мгновенно реагирует на команду).

* - Condition — это специальный тип узла в OPC UA, предназначенный для представления состояний тревог и событий.
   Он позволяет клиентам подписываться на события тревог, получать уведомления и просматривать историю тревог.
   Condition содержит различные свойства, такие как ActiveState (активна ли тревога), Severity (уровень серьезности),
   Message (сообщение тревоги) и другие, которые помогают в управлении и мониторинге состояния системы.

   - Severity — это числовое значение, которое указывает на уровень серьезности тревоги. Определяется в диапазоне от 1 до 1000,
   где 1 — наименее серьезная тревога, а 1000 — наиболее серьезная. Клиенты OPC UA могут использовать это значение
   для фильтрации и приоритизации тревог в пользовательском интерфейсе.

   - Гистерезис — это метод предотвращения частого переключения состояний тревог при колебаниях значения PV
   вокруг пороговых значений. Например, если порог тревоги High установлен на 80, а гистерезис равен 2,
   то тревога High активируется при достижении PV 80, но деактивируется только при снижении PV до 78.

   - Политика безопасности клапана определяет, как должен вести себя клапан при возникновении аварийных ситуаций,
   таких как срабатывание тревог HighHigh или LowLow. В зависимости от политики,
   клапан может удерживать текущее положение, переходить в ручной режим управления
   или переходить в безопасное положение (safe output).
*/

void tick(UA_Server* server, void* ctx)
{
    if (!server || !ctx) return;

    UA_Double ctrl;
    ControlLoop* loop = (ControlLoop*)ctx;

    read_ds18b20(&loop->sensor.io);

    const UA_Double     pv = loop->sensor.io.pv;
    const UA_StatusCode pvSt = loop->sensor.io.st;
    const UA_Boolean    pvGood = UA_StatusCode_isGood(pvSt);

    if (pvGood && loop->pid.mode) {
        loop->pid.processvalue = pv;
        pidCalculate(&loop->pid);
        ctrl = loop->pid.output;
    }
    else {
        ctrl = loop->pid.manualoutput;
    }

    if (!UA_NodeId_isNull(&loop->sensor.alarmConditionId)) {
        if (!pvGood) {
            update_active_flag(server, loop);
            set_severity(server, loop);
            set_message_alarm(server, loop); // ЗАМЕТКА. Создать функцию для сообщения о плохом PV.

            UA_StatusCode rc = UA_Server_triggerConditionEvent(server, loop->sensor.alarmConditionId, loop->sensor.objId, NULL);
            if (rc != UA_STATUSCODE_GOOD) printf("TriggerConditionEvent (bad PV) failed: %s\n", UA_StatusCode_name(rc));
        }
        else {
            if (update_alarm_state_with_hyst(pv, &loop->sensor.limits, &loop->sensor.state)) {
                update_active_flag(server, loop);
                set_severity(server, loop);
                set_message_alarm(server, loop);

                UA_StatusCode rc = UA_Server_setLimitState(server, loop->sensor.alarmConditionId, pv);
                if (rc != UA_STATUSCODE_GOOD) printf("SetLimitState failed: %s\n", UA_StatusCode_name(rc));
                rc = UA_Server_triggerConditionEvent(server, loop->sensor.alarmConditionId, loop->sensor.objId, NULL);
                if (rc != UA_STATUSCODE_GOOD) printf("TriggerConditionEvent failed: %s\n", UA_StatusCode_name(rc));
            }
        }
    }

    emergency_protection(loop, &ctrl);

    if (loop->valve.clampEnable) {
        UA_Double lo = loop->valve.outMin;
        UA_Double hi = loop->valve.outMax;
        if (hi < lo) { UA_Double t = lo; lo = hi; hi = t; } // авто-починка конфигурации
        ctrl = clampd(ctrl, lo, hi);
        printf("Значение клапана после ограничения clampd: %.2f, ctrl\n");
    }

    printf("Передаем степень открытия клапана %.2f в command и actual pos\n", ctrl);
    loop->valve.command = ctrl;
    loop->valve.actual_position = ctrl;
    actuator_valve_send(loop->valve.command);
}