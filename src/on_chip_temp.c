#include <bluetooth/gatt.h>
#include <nrf.h>
#include <stdio.h>

#include "on_chip_temp.h"

#define SYS_LOG_DOMAIN "on_chip_temp"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

s16_t on_chip_temp_get(void)
{
	NRF_TEMP->TASKS_START = 1;
	while (!NRF_TEMP->EVENTS_DATARDY)
		;
	return NRF_TEMP->TEMP * 25;
}

void on_chip_temp_init(void)
{
    on_chip_temp_update();
}

void on_chip_temp_update(void)
{
	on_chip_temp = on_chip_temp_get();
}
