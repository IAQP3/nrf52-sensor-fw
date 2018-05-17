#include <nrf.h>
#include <sensor.h>
#include <stdio.h>

#include "hts221_bt.h"
#include "vdd.h"
#include "meas_rates.h"

#define SYS_LOG_DOMAIN "HTS221_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

static struct device *dev;

void hts221_bt_init(void)
{

	dev = device_get_binding("HTS221");
	if (!dev) {
		SYS_LOG_ERR("Failed to get HTS221 device binding\n");
		return;
	}

	/* There's no need to rerun the device init on HTS221 */
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
	SYS_LOG_INF("temp: %d, humid: %d", t.val1, h.val1);
}

static void hts221_bt_thread(void *p1, void *p2, void *p3)
{
	for (;;) {
		vdd_get();
		hts221_bt_init();
		hts221_bt_update();
		vdd_put();
		k_sleep(meas_intervals->hts221);
	}
}

K_THREAD_DEFINE(hts221_bt_thd, 512, hts221_bt_thread, NULL, NULL, NULL,
		10, 0, K_NO_WAIT);
