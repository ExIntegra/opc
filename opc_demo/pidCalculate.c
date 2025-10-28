#include "DAQ.h"

void pidCalculate(PIDControllerType* pid) {
    /* шаг дискретизации тика (сейчас 100 мс в UA_Server_addRepeatedCallback) */
    const double dt = 0.1;     // секунды

    /* 1) ошибка */
    const double error = pid->setpoint - pid->processvalue;

    /* 2) пропорциональная часть */
    const double p = pid->kp * error;

    /* 3) интегральная часть (с учетом dt) */
    pid->integral += error * dt;

    /* простой анти-windup: ограничим интеграл разумным коридором */
    const double iMin = -1e6, iMax = 1e6;   // при желании вынесите в настройки
    if (pid->integral > iMax) pid->integral = iMax;
    if (pid->integral < iMin) pid->integral = iMin;

    const double i = pid->ki * pid->integral;

    /* 4) производная по ошибке (разностная, с учетом dt) */
    const double d = (dt > 0.0) ? pid->kd * ((error - pid->lastError) / dt) : 0.0;
    pid->lastError = error;

    /* 5) суммарный выход до внешних ограничений клапана */
    pid->output = p + i + d;

    /*
    printf("output: %f\n", pid->output);
	printf("processvalue: %f\n", pid->processvalue);
	printf("setpoint: %f\n", pid->setpoint);
    printf("P: %f, I: %f, D: %f\n", p, i, d);
	printf("-----\n");*/

}

