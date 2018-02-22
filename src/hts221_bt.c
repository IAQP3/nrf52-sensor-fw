#include <bluetooth/gatt.h>
#include <nrf.h>
#include <sensor.h>
#include <stdio.h>

#include "bt_gatt_read.h"

#define SYS_LOG_DOMAIN "HTS221_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

static s16_t temp;
static u16_t humid;
struct device *dev;

static struct bt_gatt_attr hts221_bt_ess_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_TEMPERATURE, BT_GATT_PERM_READ, read_u16,
			   NULL, &temp),
	BT_GATT_CUD("Temperature", BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_HUMIDITY, BT_GATT_PERM_READ, read_u16,
			   NULL, &humid),
	BT_GATT_CUD("Humidity", BT_GATT_PERM_READ),
};

static struct bt_gatt_service hts221_bt_ess_svc =
		BT_GATT_SERVICE(hts221_bt_ess_attrs);

void hts221_bt_update(void)
{
	struct sensor_value h, t;
	int err;

	if (!dev)
		return;

	err = sensor_sample_fetch(dev);
	if (err) {
		SYS_LOG_ERR("HTS221 sample fetch failed\n");
		return;
	}

	sensor_channel_get(dev, SENSOR_CHAN_TEMP, &t);
	sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &h);

	temp = sensor_value_to_double(&t) * 100;
	humid = sensor_value_to_double(&h) * 100;
}

void hts221_bt_init(void)
{
	int err;

	dev = device_get_binding("HTS221");
	if (!dev) {
		SYS_LOG_ERR("Failed to get HTS221 device binding\n");
		return;
	}

	err = sensor_sample_fetch(dev);
	if (err) {
		SYS_LOG_ERR("HTS221 sample fetch failed\n");
		return;
	}

	err = bt_gatt_service_register(&hts221_bt_ess_svc);
	if (err) {
		SYS_LOG_ERR("Registering HTS221 GATT services failed: %d\n", err);
		return;
	}
}
