#include <open62541/server.h>
#include "DAQ.h"
#include "opcuaSettings.h"


ControlLoop controlLoop;
UA_UInt64 PID_1 = 0;

int main(void) {

    UA_Server* server = UA_Server_new();
	UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    pid_init(&controlLoop.pid);
	sensor_init(&controlLoop.sensor);
	valve_init(&controlLoop.valve);
	addPIDControllerType(server); // Добавляем тип PIDControllerType
    addSensorType(server);        // Добавляем тип SensorType
	addValveType(server);         // Добавляем тип ValveType
	opcua_create_pid_instance(server, "PID", &controlLoop.pid);            // Создаем экземпляр PID контроллера
	opcua_create_sensor_instance(server, "Sensor", &controlLoop.sensor);   // Создаем экземпляр датчика
    opcua_create_valve_instance(server, "Valve", &controlLoop.valve);      // Создаем экземпляр клапана
	UA_Server_addRepeatedCallback(server, tick, &controlLoop, 100, &PID_1); // Добавляем колбэк tick с интервалом 100 мс
	UA_Server_runUntilInterrupt(server); // Запускаем сервер до прерывания
	UA_Server_delete(server); // Удаляем сервер
    return 0;
}
