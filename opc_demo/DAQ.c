#include "DAQ.h"
#include "sensorsRead.h"


// ���������� ������ � ������� ����������� DS18B20
void daq_tick(CashSensor* sensor) {

    double value;

    UA_StatusCode rc = ds18b20_readC(&value);
    if (rc == UA_STATUSCODE_GOOD) {
		sensor->pv = value; // ��������� �������� PV
		sensor->st = UA_STATUSCODE_GOOD; // ��������� ������ �� GOOD
    }
    else {
		sensor->st = rc; // ��������� ������ ������
    }
}

/* ������: ��� � 100�� ���������� ������ � ������������� pid1 */
void tick(UA_Server* server, void* ctx) {
	(void)server; // �� ������������
}