#ifndef BATTERY_VOLTAGE_H
#define BATTERY_VOLTAGE_H

#include <zephyr/types.h>

#define BATTERY_VOLTAGE_MEAS_INTERVAL 1000

u16_t battery_voltage_get(void);
void battery_voltage_init(void);
void battery_voltage_update(void);
u16_t battery_voltage;

#endif
