#pragma once
#include <open62541/types.h>

typedef struct {
	UA_NodeId objId;
	UA_Double volume;
} Reactor;

typedef struct {
	UA_Double manualoutput;
	UA_NodeId objId;
} ValveHandleControl;

typedef struct {
    UA_Double pv;
    UA_NodeId objId;
} Sensor;

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
        * sensorT,
        * sensorF,
        * sensorConcentrationA,
        * sensorConcentrationB;
	ValveHandleControl
        * valveRegulationConcentrationA,
        * valveRegulationQ,
        * valveRegulationT;

} ModelCtx;