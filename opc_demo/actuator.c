#include "actuator.h"

// НЕ ИСПОЛЬЗУЕТСЯ В ПРОЕКТЕ
void actuator_valve_send(UA_Double command) { (void)command; }

UA_Double clampd(UA_Double x, UA_Double lo, UA_Double hi) {
    return (x < lo) ? lo : (x > hi) ? hi : x;
}