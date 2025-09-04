#include "sensorsRead.h"
#include "sensorsAddress.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Чтение температуры с DS18B20, °C в outC, код статуса
UA_StatusCode ds18b20_readC(double* outC) {
    FILE* f = fopen(DS18B20, "r");
    if (!f) return UA_STATUSCODE_BADNOTCONNECTED;

    char l1[128], l2[128];
    if (!fgets(l1, sizeof l1, f) || !fgets(l2, sizeof l2, f)) {
        fclose(f); return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    fclose(f);

    if (!strstr(l1, "YES"))
        return UA_STATUSCODE_BADDATAUNAVAILABLE;

    char* p = strstr(l2, "t=");
    if (!p) return UA_STATUSCODE_BADDATAUNAVAILABLE;

    long milli = strtol(p + 2, NULL, 10);
    *outC = (double)milli / 1000.0;
    return UA_STATUSCODE_GOOD;
}
