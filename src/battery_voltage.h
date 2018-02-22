#ifndef BATTERY_VOLTAGE_H
#define BATTERY_VOLTAGE_H

#include <sys/types.h>

u32_t battery_voltage_get(void);
void battery_voltage_init(void);
void battery_voltage_update(void);

#endif
