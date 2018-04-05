#include <nrf.h>
#include <sensor.h>
#include <stdio.h>

#include "hts221_bt.h"

#define SYS_LOG_DOMAIN "HTS221_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

struct device *dev;

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

	hts221_bt_temp = sensor_value_to_double(&t) * 100;
	hts221_bt_humid = sensor_value_to_double(&h) * 100;
}

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
