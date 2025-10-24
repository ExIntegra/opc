#pragma once
#include <open62541/server.h>
#include <open62541/types.h>

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

/* --- Параметры клапана --- */
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

/* --- Кэш для хранения значения датчика и статуса прочтения --- */
typedef struct {
    double pv;
    UA_StatusCode st;     /* статус последнего чтения */
} CashSensor;

/* --- Параметры датчика --- */
typedef struct {
	UA_String name;
    CashSensor io;
    AlarmLimits limits;
    AlarmState state;
    UA_NodeId objId;            // NodeId объекта датчика в адресном пространстве
    UA_NodeId alarmConditionId; // NodeId ConditionType для сигнализации
} Sensor;

/* --- Параметры PID контроллера --- */
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

/* --- Полный контур управления с PID контроллером, кэшем и политикой безопасности --- */
typedef struct {
	UA_String name;
    PIDControllerType pid;
	Sensor sensor;
    Valve valve;
} ControlLoop;

/* --- Инициализируем кэш --- */
static void cash_init(CashSensor* t) {
    t->pv = 0.0;
    t->st = UA_STATUSCODE_BADDATAUNAVAILABLE;
}

/* --- Инициализация клапана --- */
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


/* --- Инициализация датчика --- */
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

/* --- Инициализация PID контроллера --- */
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

// Однократный опрос датчика в кэш.
void daq_tick(CashSensor* t);

// Многократный опрос датчика и пересчет pid.
void tick(UA_Server* server, void* ctx);

// Добавьте объявление функции PID-регулятора перед её использованием
void pidCalculate(PIDControllerType* pid);