#include <stdio.h>
#include "types.h"
#include "math_model.h"

double compute_CB(Reactor reactor, Sensor sensorTemperature,
    ConfigMathModel config, Sensor sensorQ, Sensor sensorConcentrationA)
{
    const double R = config.R;
    printf("Получаю температуру...\n");
    const double T_K = sensorTemperature.io.pv + 273.15;
    printf("Получил T: %.2f\n", sensorTemperature.io.pv);
    if (!isfinite(T_K) || T_K <= 0.0) {
        printf("Invalid temperature T=%.9f\n", T_K);
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
        printf("a or b is zero: a=%.9f b=%.9f\n", a, b);
        return NAN;
    }

    const double num = 2.0 * Vr * k1 * Q * CA;
    printf("Результаты математической модели: \n");
    printf("T=%.2f Q=%.2f Vr=%.2f CA=%.2f\n",
		T_K, Q, Vr, CA);
    printf("k1=%.9f k2=%.9f a=%.9f b=%.9f num=%.9f CB=%.12f\n",
        k1, k2, a, b, num, num / (a * b));
    printf("----------------------------------------------------------");
	printf("\n\n\n");

    return num / (a * b);
}

void model_cb(UA_Server* server, void* data) {
    (void)server;
    ModelCtx* m = (ModelCtx*)data;
    m->sensorConcentrationA->io.pv = (m->valveRegulationConcentrationA->manualoutput) / 100.0;
	m->sensorF->io.pv = valve_characteristic(m->valveRegulationQ->manualoutput);
    m->sensorConcentrationA->io.pv = valve_characteristicCA(m->valveRegulationConcentrationA->manualoutput);
    double y = compute_CB(*m->reactor, *m->sensorTemperature, m->cfg, *m->sensorF, *m->sensorConcentrationA);

    if (isfinite(y) && y >= 0.0)
        m->sensorConcentrationB->io.pv = y;
}

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
        double x = u / 70.0;          // 0..1
        return 0.7 * x * x;
    }
    else {
        double x = (u - 70.0) / 30.0; // 0..1
        return 0.7 + 0.2 * x;
    }
}