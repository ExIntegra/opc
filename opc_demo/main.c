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
	addPIDControllerType(server); // ��������� ��� PIDControllerType
    addSensorType(server);        // ��������� ��� SensorType
	addValveType(server);         // ��������� ��� ValveType
	opcua_create_pid_instance(server, "PID", &controlLoop.pid);            // ������� ��������� PID �����������
	opcua_create_sensor_instance(server, "Sensor", &controlLoop.sensor);   // ������� ��������� �������
    opcua_create_valve_instance(server, "Valve", &controlLoop.valve);      // ������� ��������� �������
	UA_Server_addRepeatedCallback(server, tick, &controlLoop, 100, &PID_1); // ��������� ������ tick � ���������� 100 ��
	UA_Server_runUntilInterrupt(server); // ��������� ������ �� ����������
	UA_Server_delete(server); // ������� ������
    return 0;
}
