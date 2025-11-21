#include "config.h"

const int config_dt = 1000;  // callback every 1000 ms (1 s)

// open62541 will store actual callback IDs here
UA_UInt64 cbModelId = 0;
UA_UInt64 cbTickId = 0;

// Global model objects
Reactor reactor;

ValveHandleControl
	valveRegulationQ,
	valveRegulationT,
	valveRegulationConcentrationA;

ModelCtx modelCtx;

Sensor
	sensorF,
	sensorT,
	sensorConcentrationA,
	sensorConcentrationB;