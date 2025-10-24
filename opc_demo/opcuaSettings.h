#pragma once

#include <open62541/plugin/log_stdout.h>
#include <open62541/types.h>
#include <math.h>
#include <open62541/server.h>
#include "DAQ.h"
#include <stdio.h>
#include <open62541/server_config_default.h>


UA_NodeId addPIDControllerType(UA_Server* server);
UA_NodeId addSensorType(UA_Server* server);
UA_NodeId addValveType(UA_Server* server);

UA_StatusCode opcua_create_pid_instance(UA_Server* server, const char* pidName, PIDControllerType* loop);
UA_StatusCode opcua_create_sensor_instance(UA_Server* server, const char* sensorName, Sensor* sensor);
UA_StatusCode opcua_create_valve_instance(UA_Server* server, const char* valveName, Valve* valve);