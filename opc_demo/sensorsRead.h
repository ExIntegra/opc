#pragma once
#include <open62541/types.h>
#include "sensorsAddress.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"

UA_StatusCode ds18b20_readC(double* outC);
void read_ds18b20(CashSensor* sensor);
void read_ds18b20_test(CashSensor* sensor, int minTemp, int maxTemp);