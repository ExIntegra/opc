#pragma once
#include <open62541/types.h>

UA_StatusCode ds18b20_readC(double* outC);  /* °C в outC, код статуса */

