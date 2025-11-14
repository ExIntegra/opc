#include "sensorsRead.h"
#include <stdio.h>

// Чтение температуры с DS18B20, °C в outC, получаем код статус прочтения датчика
UA_StatusCode ds18b20_readC(double* outC) {
    FILE* f = fopen(DS18B20, "r");
    if (!f) {
        printf("Значения датчика температуры ds18b20 не получены:");
        printf("UA_STATUSCODE_BADNOTCONNECTED. NOT OPEN FILE\n");
        return UA_STATUSCODE_BADNOTCONNECTED;
    }

    char l1[128], l2[128];
    if (!fgets(l1, sizeof l1, f) || !fgets(l2, sizeof l2, f)) {
        fclose(f);
        printf("Значения датчика температуры ds18b20 не получены:");
        printf("UA_STATUSCODE_BADUNEXPECTEDERROR\n");
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    fclose(f);

    if (!strstr(l1, "YES"))
        return UA_STATUSCODE_BADDATAUNAVAILABLE;

    char* p = strstr(l2, "t=");
    if (!p) return UA_STATUSCODE_BADDATAUNAVAILABLE;

    long milli = strtol(p + 2, NULL, 10);
    *outC = (double)milli / 1000.0;
    printf("DS18B20: %.3f C\n", *outC);
    return UA_STATUSCODE_GOOD;
}

// Функция чтения температуры с DS18B20 и обновления кэша CashSensor
void read_ds18b20(CashSensor* sensor) {
    UA_Double value;
    UA_StatusCode rc = ds18b20_readC(&value);
    if (rc == UA_STATUSCODE_GOOD) {
        sensor->pv = value; // Обновляем значение PV
        sensor->st = UA_STATUSCODE_GOOD; // Обновляем статус на GOOD
        printf("Значения записаны, статус записи GOOD.");

    }
    else {
        sensor->st = rc; // Обновляем статус ошибки
        printf("Ошибка чтения датчика. Статус обновлен.");

    }
}