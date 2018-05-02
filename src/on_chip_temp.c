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

void on_chip_temp_update(void)
{
	on_chip_temp = on_chip_temp_get();
}

static void on_chip_temp_thread(void *p1, void *p2, void *p3)
{
	for (;;) {
		on_chip_temp_update();
		k_sleep(ON_CHIP_TEMP_MEAS_INTERVAL);
	}
}

K_THREAD_DEFINE(on_chip_temp_thd, 512, on_chip_temp_thread, NULL, NULL, NULL,
		10, 0, K_NO_WAIT);
