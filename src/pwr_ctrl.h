#ifndef PWR_CTRL_H
#define PWR_CTRL_H

#include <device.h>
#include <gpio.h>

struct pwr_ctrl_pin {
	const char *port;
	u32_t pin;
	u32_t cfg;
};

struct pwr_ctrl_pin pwr_ctrl_vdd_pin = {
	.port = CONFIG_GPIO_NRF5_P0_DEV_NAME,
	.pin = 30,
	.cfg = GPIO_DIR_OUT | GPIO_PUD_PULL_UP,
};

static int pwr_ctrl_init(struct device *dev)
{
	const struct pwr_ctrl_pin *pin = dev->config->config_info;
	struct device *gpio;
	int ret;

	gpio = device_get_binding(pin->port);
	if (!gpio) {
		printf("Could not bind device \"%s\"\n", pin->port);
		return -ENODEV;
	}

	ret = gpio_pin_configure(gpio, pin->pin, pin->cfg);
	if (ret)
		return ret;

	ret = gpio_pin_write(gpio, pin->pin, 1);
	if (ret)
		return ret;

	k_sleep(10); /* Wait for the rail to compe up and stabilize */

	return 0;
}

DEVICE_INIT(pwr_ctrl_vdd, "PWR_CTRL_VDD", pwr_ctrl_init, NULL,
	    &pwr_ctrl_vdd_pin, POST_KERNEL, 50);

#endif
