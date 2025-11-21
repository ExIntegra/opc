#pragma once
#include "types.h"

void sensor_init(Sensor* s);
void reactor_init(Reactor* r);
void valve_handle_control_init(ValveHandleControl* vhc);
void model_init(ModelCtx* m, Sensor* sensorTemperature, Sensor* sensorF, Sensor* sensorConcentrationA,
	Sensor* sensorConcentrationB, Reactor* reactor, ValveHandleControl* valveRegulationConcentrationA,
	ValveHandleControl* valveRegulationQ, ValveHandleControl* valveRegulationT);