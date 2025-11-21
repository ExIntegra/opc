#pragma once
#include "types.h"

// Math model call period, ms
extern const int config_dt;

// OPC UA callback identifiers
extern UA_UInt64 cbModelId;
extern UA_UInt64 cbTickId;

// Global model objects
extern Reactor reactor;

extern ValveHandleControl
	valveRegulationQ,
	valveRegulationT,
	valveRegulationConcentrationA;

extern ModelCtx modelCtx;

extern Sensor
	sensorF,
	sensorT,
	sensorConcentrationA,
	sensorConcentrationB;
