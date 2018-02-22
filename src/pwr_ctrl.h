#ifndef PWR_CTRL_H
#define PWR_CTRL_H

#include <device.h>
#include <gpio.h>

struct pwr_ctrl_pin {
	const char *port;
	u32_t pin;
	u32_t cfg;
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

	k_sleep(100); /* Wait for the rail to compe up and stabilize */

	return 0;
}

#ifdef CONFIG_BOARD_NRF52_PCA20020
static const struct pwr_ctrl_pin pwr_ctrl_vdd_pin = {
	.port = CONFIG_GPIO_NRF5_P0_DEV_NAME,
	.pin = 30,
	.cfg = GPIO_DIR_OUT | GPIO_PUD_PULL_UP,
};

DEVICE_INIT(pwr_ctrl_vdd, "PWR_CTRL_VDD", pwr_ctrl_init, NULL,
	    &pwr_ctrl_vdd_pin, POST_KERNEL, 50);

static const struct pwr_ctrl_pin pwr_ctrl_ccs_pin = {
	.port = CONFIG_GPIO_SX1509B_DEV_NAME,
	.pin = 10,
	.cfg = GPIO_DIR_OUT,
};

DEVICE_INIT(pwr_ctrl_ccs, "PWR_CTRL_CCS", pwr_ctrl_init, NULL,
	    &pwr_ctrl_ccs_pin, POST_KERNEL, 80);

/* TODO Add reset pin handling to the driver */
static const struct pwr_ctrl_pin pwr_ctrl_ccs_reset = {
	.port = CONFIG_GPIO_SX1509B_DEV_NAME,
	.pin = 11,
	.cfg = GPIO_DIR_OUT,
};

DEVICE_INIT(pwr_ctrl_ccs_reset, "PWR_CTRL_CCS_RESET", pwr_ctrl_init, NULL,
	    &pwr_ctrl_ccs_reset, POST_KERNEL, 81);
#endif

#endif
