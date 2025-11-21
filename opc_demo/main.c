/*
 * @file main.c
 * @brief Entry point for the OPC UA server for the reactor math model.
 *
 * This program creates and runs an OPC UA server using open62541.
 * It performs the following steps:
 *   1. Creates a UA_Server instance.
 *   2. Initializes in-memory objects: sensors, valves, reactor, and the
 *      mathematical model context.
 *   3. Registers custom OPC UA types for sensors, reactor, math model,
 *      and valve handle control in the server’s address space.
 *   4. Creates logical folders ("Model", "Valves", "Sensors", "Reactors")
 *      and instantiates the corresponding OPC UA nodes bound to the
 *      initialized C objects.
 *   5. Registers a periodic callback (model_cb) with period config_dt
 *      to execute the mathematical model and update tags.
 *   6. Starts the server’s main loop and runs it until an interrupt
 *      (e.g. SIGINT) is received, then shuts down and frees resources.
 *
 * The process runs in the foreground and terminates only on interrupt
 * or fatal error from UA_Server_runUntilInterrupt.
 */

#include <open62541/server.h>
#include "init.h"
#include "types.h"
#include "config.h"
#include "math_model.h"
#include "opcuaSettings.h"

int main(void) {
    UA_Server* server = UA_Server_new();

	sensor_init(&sensorT);
	sensor_init(&sensorF);
	sensor_init(&sensorConcentrationA);
	sensor_init(&sensorConcentrationB);

	valve_handle_control_init(&valveRegulationQ);
	valve_handle_control_init(&valveRegulationT);
	valve_handle_control_init(&valveRegulationConcentrationA);

	reactor_init(&reactor);

	model_init(&modelCtx,
		&sensorT,
		&sensorF,
		&sensorConcentrationA,
		&sensorConcentrationB,
		&reactor,
		&valveRegulationConcentrationA,
		&valveRegulationQ,
		&valveRegulationT);


	addSensorType(server);
	addReactorType(server);        
	addMathModelType(server);
	addValveHandleControlType(server);

	UA_NodeId MODEL = UA_NODEID_NULL;
	UA_NodeId VALVES = UA_NODEID_NULL;
	UA_NodeId SENSORS = UA_NODEID_NULL;
	UA_NodeId REACTORS = UA_NODEID_NULL;

	opc_ua_create_cell_folder(server, "Model", &MODEL);
	opc_ua_create_cell_folder(server, "Valves", &VALVES);
	opc_ua_create_cell_folder(server, "Sensors", &SENSORS);
	opc_ua_create_cell_folder(server, "Reactors", &REACTORS);

	opc_ua_create_reactor_instance(server, REACTORS, "1-F", &reactor);
	opc_ua_create_math_model_instance(server, MODEL, "Config", &modelCtx);
	opc_ua_create_sensor_instance(server, SENSORS, "FRA-1", UA_FALSE, &sensorF);
	opc_ua_create_sensor_instance(server, SENSORS, "TRA-1", UA_FALSE, &sensorT);
	opc_ua_create_sensor_instance(server, SENSORS, "CRA-1", UA_FALSE, &sensorConcentrationA);
	opc_ua_create_sensor_instance(server, SENSORS, "CRA-2", UA_FALSE, &sensorConcentrationB);
	opc_ua_create_valve_handle_control(server, VALVES, "HC-1", &valveRegulationConcentrationA);
	opc_ua_create_valve_handle_control(server, VALVES, "HC-2", &valveRegulationQ);
	opc_ua_create_valve_handle_control(server, VALVES, "HC-3", &valveRegulationT);

	UA_Server_addRepeatedCallback(server, model_cb, &modelCtx, config_dt, &cbModelId);
	UA_Server_runUntilInterrupt(server);
	UA_Server_delete(server);
    return 0;
}
