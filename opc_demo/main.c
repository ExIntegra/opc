#include <open62541/server.h>
#include "DAQ.h"
#include "pidCalculate.h"


ControlLoop PRCA1;
UA_UInt64 PRCA_h1 = 0;
int main(void) {

    UA_Server* server = UA_Server_new();
    PID_init(&PRCA1.pid);
    cash_init(&PRCA1.cash);
    UA_Server_addRepeatedCallback(server, tick100ms_cb, &PRCA1, 1000, &PRCA_h1);
    UA_Server_runUntilInterrupt(server);
    UA_Server_delete(server);
    return 0;
}
