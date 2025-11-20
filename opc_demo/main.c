#include <open62541/server.h>
#include "types.h"
#include "opcuaSettings.h"
#include "config.h"
#include "DAQ.h"
#include "init.h"
#include <open62541/server_config_default.h>
#include "math_model.h"

ControlLoop controlLoop;
Reactor reactor;
ValveHandleControl valveRegulationConcentrationA, valveRegulationQ, valveRegulationT;
ModelCtx modelCtx;
Sensor /*sensorTemperature,*/ sensorConcentrationA, sensorF, sensorConcentrationB, sensorT;
UA_UInt64 cbModelId = 10000, cbTickId = 10000;

/* Закомментированные строки кода относятся к ПИД-регулированию, который
не задействован в курсовой работе.
Из-за этого не задойствованы следующие модули программы:
	actuator.c/.h
	alarms_utils.c/.h
	DAQ.c/.h
	pidCalculate.c/.h 
	sensorsAddress.c/.h
	sensorsRead.c/.h
*/
int main(void) {

    UA_Server* server = UA_Server_new();
	UA_ServerConfig* cfg = UA_Server_getConfig(server);

    pid_init(&controlLoop.pid);

	sensor_init(&controlLoop.sensor);
	sensor_init(&sensorConcentrationA);
	sensor_init(&sensorT);

	valve_init(&controlLoop.valve);
	valve_handle_control_init(&valveRegulationConcentrationA);
	valve_handle_control_init(&valveRegulationQ);
	valve_handle_control_init(&valveRegulationT);

	reactor_init(&reactor);

	model_init(&modelCtx, &sensorT/*&controlLoop.sensor*/, &sensorF, &sensorConcentrationA,
		&sensorConcentrationB, &reactor, &valveRegulationConcentrationA, &valveRegulationQ,
		&valveRegulationT);


	//addPIDControllerType(server); 
    addSensorType(server);        
	addValveType(server);         
	addReactorType(server);        
	addValveHandleControlType(server);
	addMathModelType(server);

	//UA_NodeId cellFolderId = UA_NODEID_NULL;
	UA_NodeId REACTORS = UA_NODEID_NULL;
	UA_NodeId VALVES = UA_NODEID_NULL;
	UA_NodeId SENSORS = UA_NODEID_NULL;
	UA_NodeId MODEL = UA_NODEID_NULL;

	//opc_ua_create_cell_folder(server, "TRCA1", &cellFolderId);
	opc_ua_create_cell_folder(server, "Reactors", &REACTORS);
	opc_ua_create_cell_folder(server, "Valves", &VALVES);
	opc_ua_create_cell_folder(server, "Sensors", &SENSORS);
	opc_ua_create_cell_folder(server, "Model", &MODEL);

	//opc_ua_create_pid_instance(server, cellFolderId, "PID", &controlLoop.pid);
	//opc_ua_create_sensor_instance(server, cellFolderId, "Sensor", UA_TRUE,&controlLoop.sensor);
	opc_ua_create_sensor_instance(server, SENSORS, "FRA-1", UA_FALSE, &sensorF); 
	opc_ua_create_sensor_instance(server, SENSORS, "CRA-1", UA_FALSE, &sensorConcentrationA);
	opc_ua_create_sensor_instance(server, SENSORS, "CRA-2", UA_FALSE, &sensorConcentrationB);
	opc_ua_create_sensor_instance(server, SENSORS, "TRA-1", UA_FALSE, &sensorT);
    //opc_ua_create_valve_instance(server, cellFolderId, "Valve", &controlLoop.valve);      
	opc_ua_create_reactor_instance(server, REACTORS, "1-F", &reactor);
	opc_ua_create_valve_handle_control(server, VALVES, "HC-1", &valveRegulationConcentrationA);
	opc_ua_create_valve_handle_control(server, VALVES, "HC-2", &valveRegulationQ);
	opc_ua_create_valve_handle_control(server, VALVES, "HC-3", &valveRegulationT);
	opc_ua_create_math_model_instance(server, MODEL, "Config", &modelCtx);

	//UA_Server_addRepeatedCallback(server, tick, &controlLoop, config_dt, &cbTickId);
	UA_Server_addRepeatedCallback(server, model_cb, &modelCtx, config_dt, &cbModelId);
	UA_Server_runUntilInterrupt(server);
	UA_Server_delete(server);
    return 0;
}
