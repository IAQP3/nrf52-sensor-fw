#include <nrf.h>
#include <stdio.h>
#include <kernel.h>

#include "battery_voltage.h"

#define SYS_LOG_DOMAIN "battery"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

void battery_voltage_init(void)
{
	battery_voltage_update();
}

u16_t battery_voltage_get(void)
{
	u32_t bat_adc;

	NRF_SAADC->ENABLE = 1;

	/* RESP VDD/2, RESN GND, GAIN: 1/4 */
	NRF_SAADC->CH[0].CONFIG = (3<<0) | (1<<4) | (2<<8);
	NRF_SAADC->CH[0].PSELP = 0;
	NRF_SAADC->CH[0].PSELN = 0;

	NRF_SAADC->RESULT.PTR = (uint32_t)&bat_adc;
	NRF_SAADC->RESULT.MAXCNT = 1;
	NRF_SAADC->RESOLUTION = 3;

	NRF_SAADC->TASKS_START = 1;
	while (!(NRF_SAADC->EVENTS_STARTED))
		;
	NRF_SAADC->TASKS_SAMPLE = 1;
	while (!(NRF_SAADC->EVENTS_END))
		;
	NRF_SAADC->TASKS_STOP = 1;

	/* 14-bit conversion, 1/4 gain, 1/2 VCC at RESP, 0.6V ref */
	return (6 * 100 * bat_adc)>>(14 - 3);
}

void battery_voltage_update(void)
{
	battery_voltage = battery_voltage_get();
}

static void battery_voltage_thread(void *p1, void *p2, void *p3)
{
	for (;;) {
		battery_voltage_update();
		k_sleep(BATTERY_VOLTAGE_MEAS_INTERVAL);
	}
}

K_THREAD_DEFINE(battery_voltage_thd, 512, battery_voltage_thread, NULL, NULL, NULL,
		10, 0, K_NO_WAIT);
