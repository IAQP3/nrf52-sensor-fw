#include <gpio.h>
#include <sensor.h>

#include "tcs34725.h"
#include "tcs34725_bt.h"
#include "vdd.h"
#include "meas_rates.h"

#define SYS_LOG_DOMAIN "TCS34725_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

#define LED_CTRL_PIN 4

static struct device *sensor;
static struct device *gpio;
static struct vdd_rail_dev rail_dev;

void tcs34725_bt_init(void)
{
	sensor = device_get_binding("TCS34725");
	if (!sensor) {
		SYS_LOG_ERR("Failed to get TCS34725H device binding");
		return;
	}

	vdd_rail_dev_call_init(&rail_dev, sensor);

	gpio = device_get_binding(CONFIG_GPIO_NRF5_P0_DEV_NAME);
	if (!gpio) {
		SYS_LOG_ERR("Failed to get GPIO device binding");
		return;
	}

	gpio_pin_configure(gpio, LED_CTRL_PIN, GPIO_DIR_OUT);
	gpio_pin_write(gpio, LED_CTRL_PIN, 0);
}

void tcs34725_bt_update(void)
{
	struct sensor_value r, g, b, l;
	int err;

	if (!sensor || !gpio)
		return;

	gpio_pin_write(gpio, LED_CTRL_PIN, 1);
	k_sleep(1000);

	err = sensor_sample_fetch(sensor);
	if (err)
		return;

	gpio_pin_write(gpio, LED_CTRL_PIN, 0);

	sensor_channel_get(sensor, SENSOR_CHAN_RED, &r);
	sensor_channel_get(sensor, SENSOR_CHAN_GREEN, &g);
	sensor_channel_get(sensor, SENSOR_CHAN_BLUE, &b);
	sensor_channel_get(sensor, SENSOR_CHAN_LIGHT, &l);

	tcs34725_bt_r = r.val1;
	tcs34725_bt_g = g.val1;
	tcs34725_bt_b = b.val1;
	tcs34725_bt_l = l.val1;

	SYS_LOG_INF("r: %d, g: %d, b: %d, l: %d", r.val1, g.val1, b.val1, l.val1);
}

static void tcs34725_bt_thread(void *p1, void *p2, void *p3)
{
	vdd_rail_dev_register(&rail_dev);

	for (;;) {
		vdd_get();
		tcs34725_bt_init();
		tcs34725_bt_update();
		vdd_put();
		k_sleep(meas_intervals->tcs34725);
	}
}

K_THREAD_DEFINE(tcs34725_bt_thd, 512, tcs34725_bt_thread, NULL, NULL, NULL,
		10, 0, K_NO_WAIT);
