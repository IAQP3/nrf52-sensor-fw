#include <bluetooth/gatt.h>
#include <nrf.h>
#include <stdio.h>

#include "bt_gatt_read.h"

#define SYS_LOG_DOMAIN "on_chip_temp"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

static s16_t temp;

static struct bt_gatt_attr on_chip_temp_bt_ess_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_TEMPERATURE, BT_GATT_PERM_READ, read_u16,
			   NULL, &temp),
	BT_GATT_CUD("On-Chip temperature", BT_GATT_PERM_READ),
};

static struct bt_gatt_service on_chip_temp_bt_ess_svc =
		BT_GATT_SERVICE(on_chip_temp_bt_ess_attrs);

s16_t on_chip_temp_get(void)
{
	NRF_TEMP->TASKS_START = 1;
	while (!NRF_TEMP->EVENTS_DATARDY)
		;
	return NRF_TEMP->TEMP * 25;
}

int on_chip_temp_init(void)
{
	int err;

	err = bt_gatt_service_register(&on_chip_temp_bt_ess_svc);
	if (err) {
		SYS_LOG_ERR("Registering nRF TEMP GATT services failed: %d", err);
		return -1;
	}
	return 0;
}

void on_chip_temp_update(void)
{
	temp = on_chip_temp_get();
}
