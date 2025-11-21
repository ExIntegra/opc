/**
 * @file init.c
 * @brief Initialization helpers for reactor, sensors, valves and model context.
 *
 * This module provides utility functions to initialize the basic data
 * structures used by the OPC UA server and the mathematical model:
 *
 *   - reactor_init() sets a default reactor volume and clears its OPC UA NodeId.
 *   - valve_handle_control_init() resets valve handle control state and NodeId.
 *   - sensor_init() clears sensor process value and NodeId.
 *   - model_init() wires together all pointers in ModelCtx and sets default
 *     kinetic parameters (R, k01, k02, EA1, EA2) and the substance ID.
 *
 * All functions only initialize existing objects provided by the caller;
 * they do not allocate or free memory.
 */

#include "init.h"

void reactor_init(Reactor* r) {
    r->objId = UA_NODEID_NULL;
    r->volume = 100;
}

void valve_handle_control_init(ValveHandleControl* vhc) {
    vhc->objId = UA_NODEID_NULL;
    vhc->manualoutput = 0.0;
}

void sensor_init(Sensor* s) {
    s->objId = UA_NODEID_NULL;
    s->pv = 0.0;
}

void model_init(ModelCtx* m, Sensor* sensorTemperature, Sensor* sensorF,
    Sensor* sensorConcentrationA, Sensor* sensorConcentrationB,
    Reactor* reactor, ValveHandleControl* valveRegulationConcentrationA,
    ValveHandleControl* valveRegulationQ, ValveHandleControl* valveRegulationT)
{
    m->reactor = reactor;
    m->valveRegulationQ = valveRegulationQ;
    m->valveRegulationConcentrationA = valveRegulationConcentrationA;
    m->valveRegulationT = valveRegulationT;
    m->sensorF = sensorF;
    m->sensorConcentrationA = sensorConcentrationA;
    m->sensorConcentrationB = sensorConcentrationB;
    m->sensorT = sensorTemperature;

    m->cfg.R = 8.314;
    m->cfg.k01 = 0;
    m->cfg.k02 = 0;
    m->cfg.EA1 = 0;
    m->cfg.EA2 = 0;

    m->substanceId = 0;
}