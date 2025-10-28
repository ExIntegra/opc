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
	UA_NodeId cellFolderId = UA_NODEID_NULL;

	opc_ua_create_cell_folder(server, "TRCA1", &cellFolderId); // ������� ����� ��� ������ TRCA1
	opc_ua_create_pid_instance(server, cellFolderId, "PID", &controlLoop.pid);              // ������� ��������� PID �����������
	opc_ua_create_sensor_instance(server, cellFolderId, "Sensor", &controlLoop.sensor);     // ������� ��������� �������
    opc_ua_create_valve_instance(server, cellFolderId, "Valve", &controlLoop.valve);        // ������� ��������� �������

	UA_Server_addRepeatedCallback(server, tick, &controlLoop, 1000, &PID_1); // ��������� ������ tick � ���������� 1000 ��
	UA_Server_runUntilInterrupt(server); // ��������� ������ �� ����������
	UA_Server_delete(server);			 // ������� ������
    return 0;
}
