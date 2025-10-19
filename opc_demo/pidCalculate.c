#include "pidCalculate.h"

void pidCalculate(PIDControllerData* pid) {

    double error = pid->setpoint - pid->processvalue;

    double proportional = pid->kp * error;

    pid->integral += error;
    double integral = pid->ki * pid->integral;

    double derivative = pid->kd * (error - pid->lastError);
    pid->lastError = error;

    pid->output = proportional + integral + derivative;

	//pid->manualoutput = pid->output; // сохраняем в ручной выход
}