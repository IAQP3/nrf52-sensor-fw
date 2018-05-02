#include <nrf.h>
#include <sensor.h>
#include <stdio.h>

#include "hts221_bt.h"

#define SYS_LOG_DOMAIN "HTS221_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

static struct device *dev;

void hts221_bt_init(void)
{
	int err;

	dev = device_get_binding("HTS221");
	if (!dev) {
		SYS_LOG_ERR("Failed to get HTS221 device binding\n");
		return;
	}

	hts221_bt_update();
}

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

	err = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &t);
	if (err)
		SYS_LOG_ERR("temp channel get failed");
	err = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &h);
	if (err)
		SYS_LOG_ERR("humid channel get failed");

	hts221_bt_temp = sensor_value_to_double(&t) * 100;
	hts221_bt_humid = sensor_value_to_double(&h) * 100;
	SYS_LOG_INF("temp: %d", t.val1);
	SYS_LOG_INF("humid: %d", h.val1);
}
