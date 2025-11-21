/**
 * @file math_model.c
 * @brief Implementation of the reactor mathematical model and OPC UA callback.
 *
 * This module provides:
 *   - The steady-state mathematical model compute_CB(), which calculates the
 *     outlet concentration CB based on reactor configuration, temperature,
 *     volumetric flow rate, and inlet concentration CA.
 *   - The periodic callback model_cb(), which is registered in the OPC UA
 *     server and:
 *       * updates sensor process values according to valve opening degree
 *         using valve_characteristic*() functions;
 *       * calls compute_CB() and writes the result to the CB sensor if valid.
 *   - Nonlinear valve characteristic functions that map manual output
 *     (0–100 %) of valves to physical quantities:
 *       * valve_characteristic()   – flow rate sensor (Q),
 *       * valve_characteristicCA() – inlet concentration CA,
 *       * valve_characteristicT()  – temperature offset for the reactor.
 *
 * All functions operate on structures provided by the caller; no dynamic
 * memory allocation is performed.
 */

#include "math_model.h"

double compute_CB(Reactor reactor, Sensor sensorTemperature,
    ConfigMathModel config, Sensor sensorQ, Sensor sensorConcentrationA)
{
    printf("\nStarting mathematical model:\n\n");
    const double R = config.R;
    const double T_K = sensorTemperature.pv + 273.15;
    if (!isfinite(T_K) || T_K <= 0.0) {
        printf("Invalid temperature T=%.2f\n", T_K);
        return NAN;
    }

    const double Q = sensorQ.pv * 1e-3 / 60.0; // m^3/s
    const double Vr = reactor.volume * 1e-3;   // m^3
    const double CA = sensorConcentrationA.pv;

    const double k1 = (config.k01 / 60.0) * exp(-config.EA1 / (R * T_K));
    const double k2 = (config.k02 / 60.0) * exp(-config.EA2 / (R * T_K));

    const double a = Vr * k1 + Q;
    const double b = Vr * k2 + Q;

    if (a == 0.0 || b == 0.0) {
        printf("Values a or b are zero: a=%.1f b=%.1f\n", a, b);
        printf("Mathematical model stopped.\n");
        printf("Possibly all valves are closed.\n");
        return NAN;
    }


    const double num = 2.0 * Vr * k1 * Q * CA;
    printf("---------------------------------------------------------------\n");
    printf("Setpoints:\n\n");
    printf("T=%.2f\nQ=%.2f\nVr=%.2f\nCA=%.2f\n", T_K, Q, Vr, CA);
    printf("k01= %.2f\n", config.k01);
    printf("k02= %.2f\n\n", config.k02);
    printf("Result:\n\n");
    printf("k1=%.9f\nk2=%.9f\na=%.9f\nb=%.9f\nnum=%.9f\nCB=%.12f\n",
        k1, k2, a, b, num, num / (a * b));
    printf("---------------------------------------------------------------\n");

    return num / (a * b);
   }

void model_cb(UA_Server* server, void* data) {
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    (void)server;
    ModelCtx* m = (ModelCtx*)data;
    printf("Valve opening degree:\n\n");
	m->sensorF->pv = valve_characteristic(m->valveRegulationQ->manualoutput);
    m->sensorConcentrationA->pv = valve_characteristicCA(m->valveRegulationConcentrationA->manualoutput);
    if (m->valveRegulationConcentrationA->manualoutput == 0.0) {
        m->sensorT->pv = 0.0;
    }
    else m->sensorT->pv = valve_characteristicT(m->valveRegulationT->manualoutput);

    printf("HC-1 %.2f\n", m->valveRegulationConcentrationA->manualoutput);
    printf("HC-2 %.2f\n", m->valveRegulationQ->manualoutput);
    printf("HC-3 %.2f\n", m->valveRegulationT->manualoutput);

    double y = compute_CB(*m->reactor, *m->sensorT, m->cfg, *m->sensorF, *m->sensorConcentrationA);

    if (isfinite(y) && y >= 0.0)
        m->sensorConcentrationB->pv = y;
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");

}

// Functions to emulate influence of valve opening degree on sensor readings
double valve_characteristic(double u) {
    if (u <= 0.0)
        return 0.0;
    if (u >= 100.0)
        return 160.0;

    if (u <= 70.0) {

        double x = u / 70.0;       
        return 144.0 * x * x;
    }
    else { 

        double x = (u - 70.0) / 30.0; 
        return 144.0 + 16.0 * x;     
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
        return -8.0;
    if (u >= 100.0)
        return 16.0;

    if (u <= 70.0) {
        double x = u / 70.0;       
        return -8.0 + 20.0 * x * x;  
    }
    else {
        double x = (u - 70.0) / 30.0;
        return 12.0 + 4.0 * x;       
    }
}
