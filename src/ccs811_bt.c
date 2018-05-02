#include <bluetooth/gatt.h>
#include <nrf.h>
#include <sensor.h>
#include <stdio.h>
#include <drivers/sensor/ccs811.h>

#include "ccs811_bt.h"

#define SYS_LOG_DOMAIN "CCS811_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

static struct device *dev;

void ccs811_bt_update(void)
{
	struct sensor_value co2_val, voc_val;
	int err;
	int retry_count;

	if (!dev)
		return;

	for (retry_count = 0; retry_count < 10; ++retry_count) {
		err = sensor_sample_fetch(dev);
		if (err)
			continue;
		sensor_channel_get(dev, SENSOR_CHAN_CO2, &co2_val);
		if (!co2_val.val1)
			continue;
		break;
	}
	if (retry_count == 10) {
		SYS_LOG_ERR("CCS811 sample fetch failed");
		return;
	}

	sensor_channel_get(dev, SENSOR_CHAN_CO2, &co2_val);
	sensor_channel_get(dev, SENSOR_CHAN_VOC, &voc_val);

	ccs811_bt_co2 = co2_val.val1;
	ccs811_bt_voc = voc_val.val1;

	SYS_LOG_INF("co2: %d, voc: %d", ccs811_bt_co2, ccs811_bt_voc);
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
			continue;
		}
		break;
	}
	if (retry_count == 0)
		SYS_LOG_ERR("CCS811 sample fetch failed");

	ccs811_bt_update();
}

static void ccs811_bt_thread(void *p1, void *p2, void *p3)
{
	for (;;) {
		if (!dev)
			ccs811_bt_init();
		ccs811_bt_update();
		k_sleep(CCS811_BT_MEAS_INTERVAL);
	}
}

K_THREAD_DEFINE(ccs811_bt_thd, 512, ccs811_bt_thread, NULL, NULL, NULL,
		10, 0, K_NO_WAIT);
