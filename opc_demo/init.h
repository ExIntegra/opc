#pragma once
#include <open62541/types.h>
#include "types.h"

void cash_init(CashSensor* t);
void valve_init(Valve* v);
void sensor_init(Sensor* s);
void pid_init(PIDControllerType* pid);
void reactor_init(Reactor* r);
void valve_handle_control_init(ValveHandleControl* vhc);
void model_init(ModelCtx* m, Sensor* sensorTemperature, Sensor* sensorF, Sensor* sensorConcentrationA,
	Sensor* sensorConcentrationB, Reactor* reactor, ValveHandleControl* valveRegulationConcentrationA,
	ValveHandleControl* valveRegulationQ, ValveHandleControl* valveRegulatoinT);