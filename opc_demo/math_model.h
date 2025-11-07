#pragma once
#include "types.h"
#include "math.h"
#include "float.h"
#include <open62541/server.h>

double compute_CB(Reactor reactor, Sensor sensorPIDTemperature, ConfigMathModel config, Sensor sensorQ, Sensor sensorConcentrationA);
void model_cb(UA_Server* server, void* data);
static double valve_characteristic(double u);