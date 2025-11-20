#pragma once

#include <open62541/types.h>

// Поведение исполняющего органа при отказе датчика.
typedef enum {
    PVFAIL_HOLD = 0,   /* держать прошлый output */
    PVFAIL_TO_MAN,     /* сразу перевести контур в MAN и взять manualoutput */
    PVFAIL_TO_SAFE     /* поставить фиксированный безопасный выход safeOutput */
} PvFailAction;

// Политика безопасности клапана.
typedef struct {
    PvFailAction action;        // выбранная стратегия
    UA_Boolean   clampEnable;   // Включить ограничение выхода
    UA_Double    outMin;        // Нижняя граница выхода (степень открытия клапана)
    UA_Double    outMax;        // Верхняя граница выхода (степень открытия клапана)
    UA_Double    safeOutput;    // Степень открытия клапана в безопасном состоянии
} SafetyPolicy;

// Пределы тревог для датчика и их приоритеты
typedef struct {
    UA_Double low;
    UA_Double lowLow;
    UA_Double high;
    UA_Double highHigh;

    UA_UInt16 priorityLowAlarm; 
    UA_UInt16 priorityHighAlarm;  
    UA_UInt16 priorityLowLowAlarm;
    UA_UInt16 priorityHighHighAlarm;
    UA_UInt16 priorityNormal;        

    UA_Double hysteresis; // Гистерезис для сигналов. Защищает от дребезга.
} AlarmLimits;

// Состояния тревог
typedef struct {
    UA_Boolean lowLow;
    UA_Boolean low;
    UA_Boolean high;
    UA_Boolean highHigh;
} AlarmState;

// Клапан управления
typedef struct {
    UA_String  name;
    UA_NodeId  objId;
    UA_Boolean clampEnable; // Включить ограничение выхода клапана

    UA_Double outMin;   // Физические пределы выхода клапана
    UA_Double outMax;   // Физические пределы выхода клапана
	UA_Double command;  // Текущая команда открытия клапана
	UA_Double actual_position; // Фактическая позиция клапана

    UA_UInt32 actionHH; // Политика безопасности при срабатывании HighHigh
    UA_UInt32 actionLL; // Политика безопасности при срабатывании LowLow
    UA_Double safeOutputHH; // Безопасный выход при срабатывании HighHigh
    UA_Double safeOutputLL; // Безопасный выход при срабатывании LowLow
} Valve;

typedef struct {
	UA_NodeId objId;  // NodeId объекта реактора в адресном пространстве
	UA_Double volume;
    UA_Boolean mixer;
	UA_String name;
} Reactor;

// Кэш датчика
typedef struct {
    UA_Double     pv;
    UA_StatusCode st;
} CashSensor;

typedef struct {
	UA_Double manualoutput;
	UA_NodeId objId;
	UA_String name;
} ValveHandleControl;

// Датчик
typedef struct {
    UA_String   name;
    UA_Boolean  alarmsEnabled;
    CashSensor  io;
    AlarmLimits limits;
    AlarmState  state;
    UA_NodeId   objId;            // NodeId объекта датчика в адресном пространстве
    UA_NodeId   alarmConditionId; // NodeId ConditionType для сигнализации
} Sensor;


// ПИД-регулятор
typedef struct {
    UA_String name;

    UA_Double kp;
    UA_Double ki;
    UA_Double kd;

    UA_Double  output;
    UA_Double  manualoutput;
    UA_Double  setpoint;
    UA_Double  processvalue;
    UA_Double  integral;
    UA_Double  lastError;
    UA_Boolean mode;
} PIDControllerType;

typedef struct {
	UA_Double k01;
	UA_Double EA1;
	UA_Double k02;
	UA_Double EA2;
    UA_Double R;
} ConfigMathModel;

typedef struct {
    Reactor* reactor;
    UA_UInt32 substanceId;
    ConfigMathModel cfg;

    Sensor
        //* sensorTemperature,
        * sensorF,
        * sensorConcentrationA,
        * sensorConcentrationB,
        * sensorT;
	ValveHandleControl
        * valveRegulationConcentrationA,
        * valveRegulationQ,
        * valveRegulationT;

} ModelCtx;

// Контрольный контур управления
typedef struct {
    UA_String         name;
    PIDControllerType pid;
    Sensor            sensor;
    Valve             valve;
} ControlLoop;