#include "pidCalculate.h"
#include <stdio.h>

// Расчет ПИД-регулятора с анти-windup
void pidCalculate(PIDControllerType* pid, UA_Double outMin, UA_Double outMax) {
    // Шаг дискретизации в секундах, config_dt в мс
    const double dt = (double)config_dt / 1000.0;

    // Авто-починка, если пределы перепутаны
    if (outMax < outMin) {
        double t = outMin;
        outMin = outMax;
        outMax = t;
    }

    /* 1) ошибка */
    const double error = pid->setpoint - pid->processvalue;

    /* 2) пропорциональная часть */
    const double p = pid->kp * error;

    /* 3) производная часть по ошибке */
    double d = 0.0;
    if (dt > 0.0) {
        d = pid->kd * ((error - pid->lastError) / dt);
    }
    pid->lastError = error;

    /* 4) кандидат на новое значение интегратора */
    double integralCandidate = pid->integral + error * dt;

    // Мягкое ограничение состояния интегратора (по модулю)
    const double iStateMax = 1000.0;   // подстрой под свою систему
    if (integralCandidate > iStateMax)  integralCandidate = iStateMax;
    if (integralCandidate < -iStateMax) integralCandidate = -iStateMax;

    // Интегральный вклад для кандидата
    double iCandidate = pid->ki * integralCandidate;

    // "Сырой" выход без учета насыщения
    double uCandidate = p + iCandidate + d;

    // Применяем насыщение по реальным пределам клапана
    double uSat = uCandidate;
    if (uSat > outMax) uSat = outMax;
    if (uSat < outMin) uSat = outMin;

    // Флаги насыщения
    const int saturatingHigh = (uCandidate > outMax);
    const int saturatingLow = (uCandidate < outMin);

    // 5) Анти-windup (conditional integration):
    //    - Если нет насыщения -> принимаем новый интеграл
    //    - Если насыщение сверху, интегрируем только если ошибка уменьшает выход (error < 0)
    //    - Если насыщение снизу, интегрируем только если ошибка увеличивает выход (error > 0)
    if (!saturatingHigh && !saturatingLow) {
        // нет насыщения
        pid->integral = integralCandidate;
    }
    else if (saturatingHigh && error < 0.0) {
        // выжимаем выше max, но ошибка хочет вернуть вниз
        pid->integral = integralCandidate;
    }
    else if (saturatingLow && error > 0.0) {
        // выжимаем ниже min, но ошибка хочет поднять вверх
        pid->integral = integralCandidate;
    }
    // иначе интеграл НЕ меняем (подавляем windup)

    // Пересчёт I и итогового выхода с новым (или старым) интегралом
    const double i = pid->ki * pid->integral;
    double u = p + i + d;
    if (u > outMax) u = outMax;
    if (u < outMin) u = outMin;

    pid->output = u;

    printf("\n\nЗначения ПИД-регулятора:\n");
    printf("kp: %.2f, ki: %.2f, kd: %.2f \n", pid->kp, pid->ki, pid->kd);
    printf("output: %f\n", pid->output);
    printf("processvalue: %f\n", pid->processvalue);
    printf("setpoint: %f\n", pid->setpoint);
    printf("P: %f, I: %f, D: %f\n\n", p, i, d);
}