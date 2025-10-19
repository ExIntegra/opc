#include <open62541/server.h>
#include "DAQ.h"
#include "pidCalculate.h"
#include "opcuaSettings.h"

ControlLoop PRCA1;
UA_UInt64 PRCA_h1 = 0;

int main(void) {

    UA_Server* server = UA_Server_new();
    PID_init(&PRCA1.pid);
    //cash_init(&PRCA1.cash);
	addPIDControllerType(server); // Добавляем тип PIDControllerType
	opcua_create_pid_instance(server, "PRCA1", &PRCA1); // Создаем экземпляр PID контроллера
    UA_Server_addRepeatedCallback(server, tick, &PRCA1, 100, &PRCA_h1);
    UA_Server_runUntilInterrupt(server);
    UA_Server_delete(server);
    return 0;
}
