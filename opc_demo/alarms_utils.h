#pragma once
#include <open62541/server.h>
#include <open62541/types.h>
#include "types.h"
#include <stdio.h>

// Проверяем, активна ли любая из тревог.
void update_active_flag(UA_Server* server, ControlLoop* loop);

// Устанавливаем уровень серьезности тревоги в зависимости от активной тревоги.
void set_severity(UA_Server* server, ControlLoop* loop);

// Устанавливаем сообщение тревоги в зависимости от активной тревоги.
void set_message_alarm(UA_Server* server, ControlLoop* loop);

// Обновляем состояние тревог с гистерезисом.
UA_Boolean update_alarm_state_with_hyst(UA_Double pv, const AlarmLimits* lim, AlarmState* st);

// Выполняем аварийное отключение контура управления.
void emergency_protection(ControlLoop* loop, UA_Double* ctrl);

// Устанавливаем значение двухсостояного идентификатора в Condition.
void set_two_state_id(UA_Server* server, const UA_NodeId cond, const char* varName, UA_Boolean state);