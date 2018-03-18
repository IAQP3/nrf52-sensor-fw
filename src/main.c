#include <stdio.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <sensor.h>
#include <gpio.h>
#include <nrf.h>

#include "on_chip_temp.h"
#include "battery_voltage.h"
#include "tcs34725.h"
#include "hts221_bt.h"
#include "ccs811_bt.h"

#define SYS_LOG_DOMAIN "main"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

#define DEVICE_NAME	CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(CONFIG_BT_DEVICE_NAME) - 1)

static const struct bt_data bt_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x05, 0x18),
};

static const struct bt_data bt_sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void bt_ready_cb(int err)
{
	if (err) {
		SYS_LOG_ERR("Bluetooth init failed: %d", err);
		return;
	}

	on_chip_temp_init();
	battery_voltage_init();
	hts221_bt_init();
	ccs811_bt_init();

	err = bt_le_adv_start(BT_LE_ADV_CONN, bt_ad, ARRAY_SIZE(bt_ad),
			      bt_sd, ARRAY_SIZE(bt_sd));
	if (err) {
		SYS_LOG_ERR("Bluetooth advertising start failed: %d", err);
		return;
	}
}

static void bt_connected_cb(struct bt_conn *conn, u8_t err)
{
	if (err) {
		SYS_LOG_ERR("Bluetooth connect failed: %d", err);
		return;
	}
	SYS_LOG_DBG("Bluetooth connected\n");
}

static void bt_disconnected_cb(struct bt_conn *conn, u8_t reason)
{
	SYS_LOG_DBG("Bluetooth disconnected: %d", reason);
}

static struct bt_conn_cb bt_conn_callbacks = {
	.connected = bt_connected_cb,
	.disconnected = bt_disconnected_cb,
};

#ifdef CONFIG_BOARD_NRF52_PCA20020
static void gpio_test(void)
{
	struct device *gpio;
	gpio = device_get_binding(CONFIG_GPIO_SX1509B_DEV_NAME);
	if (!gpio) {
		SYS_LOG_ERR("Failed to get SX1509B device binding: %s",
			    CONFIG_GPIO_SX1509B_DEV_NAME);
		return;
	}

	for (int i = 5; i < 8; ++i) {
		gpio_pin_configure(gpio, i, GPIO_DIR_OUT | GPIO_PUD_PULL_UP);
		gpio_pin_write(gpio, i, 0);
		k_sleep(500);
	}

	k_sleep(500);

	for (int i = 5; i < 8; ++i)
		gpio_pin_write(gpio, i, 1);
}
#else
static void gpio_test(void)
{
}
#endif

static void color_test(void)
{
	struct sensor_value r, g, b;
	struct device *dev;
	int err;

	dev = device_get_binding("TCS34725");
	if (!dev) {
		SYS_LOG_ERR("Failed to get TCS34725H device binding");
		return;
	}

	err = sensor_sample_fetch(dev);
	if (err)
		return;

	sensor_channel_get(dev, SENSOR_CHAN_RED, &r);
	sensor_channel_get(dev, SENSOR_CHAN_GREEN, &g);
	sensor_channel_get(dev, SENSOR_CHAN_BLUE, &b);

	SYS_LOG_INF("r: %d, g: %d, b: %d", r.val1, g.val1, b.val1);
}

void main(void)
{
	int err;

	err = bt_enable(bt_ready_cb);
	if (err)
		SYS_LOG_ERR("Bluetooth enable failed: %d", err);

	bt_conn_cb_register(&bt_conn_callbacks);

	gpio_test();

	SYS_LOG_INF("Entering measurement loop");

	for (;;) {
		k_sleep(500);
		on_chip_temp_update();
		battery_voltage_update();
		hts221_bt_update();
		ccs811_bt_update();
		color_test();
	}
}
