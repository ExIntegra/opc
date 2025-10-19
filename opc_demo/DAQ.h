#pragma once
#include <open62541/server.h>
#include <open62541/types.h>
#include "pidCalculate.h"

/* --- Режим политики безопасности клапана ---*/
typedef enum {
    PVFAIL_HOLD = 0,   /* держать прошлый output */
    PVFAIL_TO_MAN,     /* сразу перевести контур в MAN и взять manualoutput */
    PVFAIL_TO_SAFE     /* поставить фиксированный безопасный выход safeOutput */
} PvFailAction;

/* --- Политика безопасности --- */
typedef struct {
    PvFailAction action;        // выбранная стратегия
    UA_Boolean clampEnable;   // Включить ограничение выхода
    UA_Double outMin;        // Нижняя граница выхода (степень открытия клапана)
    UA_Double outMax;        // Верхняя граница выхода (степень открытия клапана)
    UA_Double safeOutput;    // Степень открытия клапана в безопасном состоянии
} SafetyPolicy;

/* --- Установка сигналов сигнализации и блокировок --- */
typedef struct {
    UA_Double low;
    UA_Double lowLow;
    UA_Double high;
    UA_Double highHigh;
    UA_Double hysteresis; // гистерезис для сигналов. Защищает от дребезга.
} AlarmLimits;

/* --- Состояние сигналов сигнализации --- */
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


/* --- Кэш для хранения значения датчика и статуса прочтения --- */
typedef struct {
    double pv;
    UA_StatusCode st;     /* статус последнего чтения */
} CashSensor;

typedef struct {
	UA_String name;
    CashSensor io;
    AlarmLimits limits;
    AlarmState state;
    UA_NodeId objId;            // NodeId объекта датчика в адресном пространстве
    UA_NodeId alarmConditionId; // NodeId ConditionType для сигнализации
} Sensor;

/* --- Полный контур управления с PID контроллером, кэшем и политикой безопасности --- */
typedef struct {
	UA_String name;
    PIDControllerData pid;
	Sensor sensor;
    Valve valve;
} ControlLoop;


/* --- Инициализируем кэш --- */
static void cash_init(CashSensor* t) {
    t->pv = 0.0;
    t->st = UA_STATUSCODE_BADDATAUNAVAILABLE;
}

// Однократный опрос датчика в кэш.
void daq_tick(CashSensor* t);

// Многократный опрос датчика и пересчет pid.
void tick(UA_Server* server, void* ctx);