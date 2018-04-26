#include <sensor.h>

#include "tcs34725.h"
#include "tcs34725_bt.h"

#define SYS_LOG_DOMAIN "TCS34725_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

struct device *dev;

void tcs34725_bt_init(void)
{
	dev = device_get_binding("TCS34725");
	if (!dev) {
		SYS_LOG_ERR("Failed to get TCS34725H device binding");
		return;
	}
}

void tcs34725_bt_update(void)
{
	struct sensor_value r, g, b, l;
	int err;

	if (!dev)
		return;

	err = sensor_sample_fetch(dev);
	if (err)
		return;

	sensor_channel_get(dev, SENSOR_CHAN_RED, &r);
	sensor_channel_get(dev, SENSOR_CHAN_GREEN, &g);
	sensor_channel_get(dev, SENSOR_CHAN_BLUE, &b);
	sensor_channel_get(dev, SENSOR_CHAN_LIGHT, &l);

	tcs34725_bt_r = r.val1;
	tcs34725_bt_g = g.val1;
	tcs34725_bt_b = b.val1;
	tcs34725_bt_l = l.val1;

	SYS_LOG_INF("r: %d, r: %d, b: %d, l: %d\n", r.val1, g.val1, b.val1,
		    l.val1);
}
