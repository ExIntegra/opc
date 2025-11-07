#pragma once

#include <open62541/plugin/log_stdout.h>
#include <open62541/types.h>
#include <math.h>
#include <open62541/server.h>
#include <stdio.h>
#include <open62541/server_config_default.h>
#include "types.h"


UA_NodeId addPIDControllerType(UA_Server* server);
UA_NodeId addSensorType(UA_Server* server);
UA_NodeId addValveType(UA_Server* server);
UA_NodeId addReactorType(UA_Server* server);
UA_NodeId addValveHandleControlType(UA_Server* server);

UA_StatusCode opc_ua_create_cell_folder(UA_Server* server, const char* cellName, UA_NodeId* outFolderId);
UA_StatusCode opc_ua_create_pid_instance(UA_Server* server, UA_NodeId parentFolder, const char* pidName, PIDControllerType* loop);
UA_StatusCode opc_ua_create_sensor_instance(UA_Server* server, UA_NodeId parentFolder, const char* sensorName, UA_Boolean enableAlarms, Sensor* sensor);
UA_StatusCode opc_ua_create_valve_instance(UA_Server* server, UA_NodeId parentFolder, const char* valveName, Valve* valve);
UA_StatusCode opc_ua_create_reactor_instance(UA_Server* server, UA_NodeId parentFolder, const char* reactorName, Reactor* reactor);
UA_StatusCode opc_ua_create_valve_handle_control(UA_Server* server, UA_NodeId parentFolder, const char* valveHandleControlName, ValveHandleControl* valveHandleControl);