#pragma once
#include "types.h"
#include <open62541/server.h>
#include <math.h>
#include <stdio.h>
#include <float.h>

static double valve_characteristic(double u);
static double valve_characteristicCA(double u);
static double valve_characteristicT(double u);

double compute_CB(Reactor reactor,
    Sensor sensorPIDTemperature,
    ConfigMathModel config,
    Sensor sensorQ,
    Sensor sensorConcentrationA);

void model_cb(UA_Server* server, void* data);
