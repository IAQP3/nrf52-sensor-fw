#include <bluetooth/gatt.h>
#include <nrf.h>
#include <sensor.h>
#include <stdio.h>
#include <gpio.h>

#include "ccs811_bt.h"
#include "vdd.h"
#include "meas_rates.h"

#define SYS_LOG_DOMAIN "CCS811_BT"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

#define CCS_VDD_PWR_CTRL_GPIO_PIN 10

static struct device *dev;
static struct device *ccs_vdd_gpio;
static struct vdd_rail_dev rail_dev;
static struct vdd_rail_dev rail_dev_gpio;

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
	static const struct sensor_value sampling_interval = { .val1 = 10,
							       .val2 = 0 };
	int retry_count;
	int err;

	dev = device_get_binding("CCS811");
	if (!dev) {
		SYS_LOG_ERR("Failed to get CCS811 device binding");
		return;
	}

	vdd_rail_dev_call_init(&rail_dev, dev);

	for (retry_count = 10; retry_count; --retry_count) {
		err = sensor_sample_fetch(dev);
		if (err) {
			continue;
		}
		break;
	}
	if (retry_count == 0)
		SYS_LOG_ERR("CCS811 sample fetch failed");

	sensor_attr_set(dev, SENSOR_CHAN_CO2, SENSOR_ATTR_SAMPLING_INTERVAL,
			&sampling_interval);
}

static void ccs811_bt_thread(void *p1, void *p2, void *p3)
{
	vdd_rail_dev_register(&rail_dev);
	vdd_rail_dev_register(&rail_dev_gpio);

	for (;;) {
		vdd_get();

		ccs_vdd_gpio = device_get_binding(CONFIG_GPIO_SX1509B_DEV_NAME);
		if (!ccs_vdd_gpio) {
			SYS_LOG_ERR("Failed to get SX1509B device binding");
			return;
		}
		vdd_rail_dev_call_init(&rail_dev_gpio, ccs_vdd_gpio);
		gpio_pin_configure(ccs_vdd_gpio, CCS_VDD_PWR_CTRL_GPIO_PIN,
				   GPIO_DIR_OUT);
		gpio_pin_write(ccs_vdd_gpio, CCS_VDD_PWR_CTRL_GPIO_PIN, 1);

		ccs811_bt_init();
		ccs811_bt_update();

		gpio_pin_write(ccs_vdd_gpio, CCS_VDD_PWR_CTRL_GPIO_PIN, 0);

		vdd_put();

		k_sleep(meas_intervals->ccs811);
	}
}

K_THREAD_DEFINE(ccs811_bt_thd, 512, ccs811_bt_thread, NULL, NULL, NULL,
		10, 0, K_NO_WAIT);
