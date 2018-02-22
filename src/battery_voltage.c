#include <bluetooth/gatt.h>
#include <nrf.h>
#include <stdio.h>

#include "battery_voltage.h"
#include "bt_gatt_read.h"

#define SYS_LOG_DOMAIN "battery"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

static u32_t battery_voltage;

static struct bt_gatt_attr battery_voltage_bt_ess_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_BAS_BATTERY_LEVEL,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_BAS_BATTERY_LEVEL, BT_GATT_PERM_READ,
			   read_u32, NULL, &battery_voltage),
	BT_GATT_CUD("Battery Voltage", BT_GATT_PERM_READ),
};

static struct bt_gatt_service battery_voltage_bt_ess_svc =
		BT_GATT_SERVICE(battery_voltage_bt_ess_attrs);

int battery_voltage_init(void)
{
	int err;

	err = bt_gatt_service_register(&battery_voltage_bt_ess_svc);
	if (err) {
		SYS_LOG_ERR("Registering nRF Battery Voltage GATT services failed: %d\n",
		       err);
		return -1;
	}
	return 0;
}

unsigned int battery_voltage_get(void)
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

