#include "DAQ.h"
#include "sensorsRead.h"


// Обновление данных с датчика температуры DS18B20
void daq_tick(CashSensor* sensor) {

    double value;

    UA_StatusCode rc = ds18b20_readC(&value);
    if (rc == UA_STATUSCODE_GOOD) {
		sensor->pv = value; // Обновляем значение PV
		sensor->st = UA_STATUSCODE_GOOD; // Обновляем статус на GOOD
    }
    else {
		sensor->st = rc; // Обновляем статус ошибки
    }
}

/* Колбэк: раз в 100мс опрашиваем датчик и пересчитываем pid1 */
void tick(UA_Server* server, void* ctx) {
	(void)server; // Не используется
}