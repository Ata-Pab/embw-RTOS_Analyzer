#ifndef SIM_HAL_H
#define SIM_HAL_H
#include <stdint.h>
#include <stdbool.h>

void sim_HAL_SetPin(uint8_t pin, bool val);
bool sim_HAL_GetPin(uint8_t pin);
uint32_t sim_HAL_GetTickUs(void); /* driven by the Qt timer */

#endif /* SIM_HAL_H */
