#include <bluetooth/gatt.h>
#include <nrf.h>
#include <sensor.h>
#include <stdio.h>

#include "bt_gatt_read.h"

#define SYS_LOG_DOMAIN "CCS811_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

static u16_t co2; /* In ppm */
static u16_t voc; /* In ppb */
struct device *dev;

#define UUID_PRIMARY	BT_UUID_DECLARE_16(0x0200)
#define UUID_CO2	BT_UUID_DECLARE_16(0x0201)
#define UUID_VOC	BT_UUID_DECLARE_16(0x0202)

static struct bt_gatt_attr ccs811_bt_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(UUID_PRIMARY),
	BT_GATT_CHARACTERISTIC(UUID_CO2,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(UUID_CO2, BT_GATT_PERM_READ, read_u16,
			   NULL, &co2),
	BT_GATT_CUD("CO2", BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(UUID_VOC,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(UUID_VOC, BT_GATT_PERM_READ, read_u16,
			   NULL, &voc),
	BT_GATT_CUD("VOC", BT_GATT_PERM_READ),
};

static struct bt_gatt_service ccs811_bt_svc = BT_GATT_SERVICE(ccs811_bt_attrs);

void ccs811_bt_update(void)
{
	struct sensor_value co2_val, voc_val;
	int err;

	if (!dev)
		return;

	err = sensor_sample_fetch(dev);
	if (err) {
		SYS_LOG_ERR("CCS811 sample fetch failed");
		return;
	}

	sensor_channel_get(dev, SENSOR_CHAN_CO2, &co2_val);
	sensor_channel_get(dev, SENSOR_CHAN_VOC, &voc_val);

	co2 = co2_val.val1;
	voc = voc_val.val1;

	SYS_LOG_INF("co2: %d, voc: %d\n", co2, voc);
}

void ccs811_bt_init(void)
{
	int err;
	int retry_count;

	dev = device_get_binding("CCS811");
	if (!dev) {
		SYS_LOG_ERR("Failed to get CCS811 device binding");
		return;
	}

	for (retry_count = 10; retry_count; --retry_count) {
		err = sensor_sample_fetch(dev);
		if (err) {
			SYS_LOG_ERR("CCS811 sample fetch failed");
			continue;
		}
		break;
	}

	err = bt_gatt_service_register(&ccs811_bt_svc);
	if (err)
		SYS_LOG_ERR("Registering CCS811 GATT services failed: %d", err);
}
