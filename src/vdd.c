#include <kernel.h>
#include <gpio.h>
#include <misc/dlist.h>

#include "vdd.h"

#define SYS_LOG_DOMAIN "VDD"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

#define VDD_PWR_CTRL_GPIO_PIN 30
#define CCS_VDD_PWR_CTRL_GPIO_PIN 10

static int vdd_count = 0;
K_MUTEX_DEFINE(vdd_mutex);
static sys_dlist_t rail_devs = { .head = &rail_devs, .tail = &rail_devs };

static void vdd_set(int state)
{
	static struct device *gpio = NULL;

	if (!gpio)
		gpio = device_get_binding(CONFIG_GPIO_NRF5_P0_DEV_NAME);

	gpio_pin_write(gpio, VDD_PWR_CTRL_GPIO_PIN, state);

	k_sleep(1);
}

void vdd_rail_dev_register(struct vdd_rail_dev *rail_dev)
{
	sys_dlist_append(&rail_devs, &rail_dev->node);
}

int vdd_rail_dev_call_init(struct vdd_rail_dev *rail_dev, struct device *dev)
{
	if (!rail_dev->need_init)
		return 0;
	dev->config->init(dev);
	rail_dev->need_init = 0;
}

void vdd_get(void)
{
	k_mutex_lock(&vdd_mutex, K_FOREVER);
	++vdd_count;
	SYS_LOG_DBG("count: %d", vdd_count);
	if (vdd_count == 1) {
		vdd_set(1);
		SYS_LOG_INF("turning rail on");
	}
	k_mutex_unlock(&vdd_mutex);
}

void vdd_put(void)
{
	struct vdd_rail_dev *rail_dev;

	k_mutex_lock(&vdd_mutex, K_FOREVER);
	--vdd_count;
	SYS_LOG_DBG("count: %d", vdd_count);
	if (vdd_count == 0) {
		vdd_set(0);
		SYS_LOG_INF("turning rail off");
		SYS_DLIST_FOR_EACH_CONTAINER(&rail_devs, rail_dev, node) {
			rail_dev->need_init = 1;
		}
	}
	k_mutex_unlock(&vdd_mutex);
}
