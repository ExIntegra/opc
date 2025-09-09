#pragma once

#include <open62541/plugin/log_stdout.h>
#include <open62541/types.h>
#include <math.h>
#include <open62541/server.h>
#include "pidCalculate.h"
#include "daq.h"
#include <stdio.h>

UA_NodeId addPIDControllerType(UA_Server* server);
UA_StatusCode opcua_create_pid_instance(UA_Server* server, const char* pidName, ControlLoop* loop);