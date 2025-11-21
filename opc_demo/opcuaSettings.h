#pragma once
#include <open62541/server.h>
#include "types.h"

UA_NodeId addSensorType(UA_Server* server);
UA_NodeId addReactorType(UA_Server* server);
UA_NodeId addMathModelType(UA_Server* server);
UA_NodeId addValveHandleControlType(UA_Server* server);

UA_StatusCode opc_ua_create_cell_folder(UA_Server* server, const char* cellName, UA_NodeId* outFolderId);

UA_StatusCode opc_ua_create_math_model_instance(UA_Server* server, UA_NodeId parentFolder,
    const char* name, ModelCtx* m);

UA_StatusCode opc_ua_create_reactor_instance(UA_Server* server, UA_NodeId parentFolder,
    const char* reactorName, Reactor* reactor);

UA_StatusCode opc_ua_create_sensor_instance(UA_Server* server, UA_NodeId parentFolder,
    const char* sensorName, UA_Boolean enableAlarms, Sensor* sensor);

UA_StatusCode opc_ua_create_valve_handle_control(UA_Server* server, UA_NodeId parentFolder,
    const char* valveHandleControlName, ValveHandleControl* valveHandleControl);