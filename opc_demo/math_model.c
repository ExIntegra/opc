#include <stdio.h>
#include "types.h"
#include "math_model.h"

double compute_CB(Reactor reactor, Sensor sensorTemperature,
    ConfigMathModel config, Sensor sensorQ, Sensor sensorConcentrationA)
{
    printf("\nЗапуск математической модели:\n");
    const double R = config.R;
    const double T_K = sensorTemperature.io.pv + 273.15;
    if (!isfinite(T_K) || T_K <= 0.0) {
        printf("Invalid temperature T=%.2f\n", T_K);
        return NAN;
    }

	const double Q = sensorQ.io.pv * 1e-3 / 60.0; // м3/с
    const double Vr = reactor.volume * 1e-3; // м3
    const double CA = sensorConcentrationA.io.pv;

    const double k1 = (config.k01 / 60.0) * exp(-config.EA1 / (R * T_K));
    const double k2 = (config.k02 / 60.0) * exp(-config.EA2 / (R * T_K));

    const double a = Vr * k1 + Q;
    const double b = Vr * k2 + Q;

    if (a == 0.0 || b == 0.0) {
        printf("a или b равны нулю: a=%.1f b=%.1f\n", a, b);
        printf("Математическая модель остановлена.\n");
        printf("Возможно все клапана закрыты.\n");
        return NAN;
    }

    const double num = 2.0 * Vr * k1 * Q * CA;
    printf("---------------------------------------------------------------\n");
    printf("Уставки:\n\n");
    printf("T=%.2f\nQ=%.2f\nVr=%.2f\nCA=%.2f\n", T_K, Q, Vr, CA);
    printf("k01= %.2f\n", config.k01);
    printf("k02= %.2f\n\n", config.k02);
    printf("Результат:\n\n");
    printf("k1=%.9f\nk2=%.9f\na=%.9f\nb=%.9f\nnum=%.9f\nCB=%.12f\n",
        k1, k2, a, b, num, num / (a * b));
    printf("---------------------------------------------------------------\n");
    return num / (a * b);
   }

void model_cb(UA_Server* server, void* data) {
    (void)server;
    ModelCtx* m = (ModelCtx*)data;
    printf("Степень открытия клапанов:\n\n");
	m->sensorF->io.pv = valve_characteristic(m->valveRegulationQ->manualoutput);
    m->sensorConcentrationA->io.pv = valve_characteristicCA(m->valveRegulationConcentrationA->manualoutput);
    if (m->valveRegulationConcentrationA->manualoutput == 0.0) {
        m->sensorT->io.pv = 0.0;
    }
    else m->sensorT->io.pv = valve_characteristicT(m->valveRegulationT->manualoutput);

    printf("HC-1 %.2f\n", m->valveRegulationConcentrationA->manualoutput);
    printf("HC-2 %.2f\n", m->valveRegulationQ->manualoutput);
    printf("HC-3 %.2f\n", m->valveRegulationT->manualoutput);

    double y = compute_CB(*m->reactor, *m->sensorT/**m->sensorTemperature*/, m->cfg, *m->sensorF, *m->sensorConcentrationA);

    if (isfinite(y) && y >= 0.0)
        m->sensorConcentrationB->io.pv = y;
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n\n\n");

}

// Функция для имитации влияния степени открытия клапана на показания датчика
static double valve_characteristic(double u) {
    if (u <= 0.0)
        return 0.0;
    if (u >= 100.0)
        return 160.0;

    if (u <= 70.0) {
        // Быстрый набор к 70%: квадратичная, ускоряющаяся к 70
        // Q(0) = 0, Q(70) = 144
        double x = u / 70.0;          // 0..1
        return 144.0 * x * x;
    }
    else { 
        // Медленный довод от 144 до 160 между 70% и 100%
        // Q(70) = 144, Q(100) = 160
        double x = (u - 70.0) / 30.0; // 0..1
        return 144.0 + 16.0 * x;      // линейно, маленький наклон
    }
}

static double valve_characteristicCA(double u) {
    if (u <= 0.0)
        return 0.0;
    if (u >= 100.0)
        return 0.9;

    if (u <= 70.0) {
        double x = u / 70.0;
        return 0.7 * x * x;
    }
    else {
        double x = (u - 70.0) / 30.0;
        return 0.7 + 0.2 * x;
    }
}

static double valve_characteristicT(double u) {
    if (u <= 0.0)
        return -8.0;   // минимум
    if (u >= 100.0)
        return 16.0;   // максимум

    if (u <= 70.0) {
        // 0..70%: от -8 до 12, ускоряющийся рост
        // T(0) = -8; T(70) = 12
        double x = u / 70.0;          // 0..1
        return -8.0 + 20.0 * x * x;   // -8 + (12 - (-8)) * x^2
    }
    else {
        // 70..100%: от 12 до 16, медленный линейный рост
        // T(70) = 12; T(100) = 16
        double x = (u - 70.0) / 30.0; // 0..1
        return 12.0 + 4.0 * x;        // 12 + (16 - 12) * x
    }
}
