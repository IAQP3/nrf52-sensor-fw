#include <bluetooth/gatt.h>
#include <nrf.h>
#include <sensor.h>
#include <stdio.h>

#include "ccs811_bt.h"

#define SYS_LOG_DOMAIN "CCS811_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

struct device *dev;

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

	ccs811_bt_co2 = co2_val.val1;
	ccs811_bt_voc = voc_val.val1;

	SYS_LOG_INF("co2: %d, voc: %d\n", ccs811_bt_co2, ccs811_bt_voc);
}

void ccs811_bt_init(void)
{
	int retry_count;
	int err;

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
	
	ccs811_bt_update();
}
