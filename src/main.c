#include <stdio.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <sensor.h>
#include <gpio.h>
#include <nrf.h>
#include <soc_power.h>
#include <board.h>

#include "tcs34725.h"
#include "on_chip_temp.h"
#include "battery_voltage.h"
#include "hts221_bt.h"
#include "ccs811_bt.h"
#include "tcs34725_bt.h"
#include "bt_gatt_read.h"
#include "meas_rates.h"

#define SYS_LOG_DOMAIN "main"
#define SYS_LOG_LEVEL SYS_LOG_LEVEL_INFO
#include <logging/sys_log.h>

#define DEVICE_NAME	CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(CONFIG_BT_DEVICE_NAME) - 1)

#define BT_SENSOR_THREAD_STACK_SIZE 256

static const struct bt_data bt_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x05, 0x18),
};

static const struct bt_data bt_sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static struct bt_gatt_attr bt_ess_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

	/* Battery voltage */
	BT_GATT_CHARACTERISTIC(BT_UUID_BAS_BATTERY_LEVEL,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_BAS_BATTERY_LEVEL, BT_GATT_PERM_READ,
			   read_u16, NULL, &battery_voltage),
	BT_GATT_CUD("Battery Voltage", BT_GATT_PERM_READ),

	/* HTS221 */
	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_TEMPERATURE, BT_GATT_PERM_READ, read_u16,
			   NULL, &hts221_bt_temp),
	BT_GATT_CUD("Temperature", BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_HUMIDITY, BT_GATT_PERM_READ, read_u16,
			   NULL, &hts221_bt_humid),
	BT_GATT_CUD("Humidity", BT_GATT_PERM_READ),

	/* CCS811 */
	BT_GATT_CHARACTERISTIC(UUID_CO2,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(UUID_CO2, BT_GATT_PERM_READ, read_u16,
			   NULL, &ccs811_bt_co2),
	BT_GATT_CUD("CO2", BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(UUID_VOC,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(UUID_VOC, BT_GATT_PERM_READ, read_u16,
			   NULL, &ccs811_bt_voc),
	BT_GATT_CUD("VOC", BT_GATT_PERM_READ),

	/* TCS34725 */
	BT_GATT_CHARACTERISTIC(UUID_RED,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(UUID_RED, BT_GATT_PERM_READ, read_u16,
			   NULL, &tcs34725_bt_r),
	BT_GATT_CUD("Red", BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(UUID_GREEN,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(UUID_GREEN, BT_GATT_PERM_READ, read_u16,
			   NULL, &tcs34725_bt_g),
	BT_GATT_CUD("Green", BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(UUID_BLUE,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(UUID_BLUE, BT_GATT_PERM_READ, read_u16,
			   NULL, &tcs34725_bt_b),
	BT_GATT_CUD("Blue", BT_GATT_PERM_READ),

	BT_GATT_CHARACTERISTIC(UUID_LIGHT,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(UUID_LIGHT, BT_GATT_PERM_READ, read_u16,
			   NULL, &tcs34725_bt_l),
	BT_GATT_CUD("Light", BT_GATT_PERM_READ),
};

static struct bt_gatt_service bt_ess_svc = BT_GATT_SERVICE(bt_ess_attrs);

static void bt_ready_cb(int err)
{
	if (err) {
		SYS_LOG_ERR("Bluetooth init failed: %d", err);
		return;
	}

	err = bt_gatt_service_register(&bt_ess_svc);

	if (err) {
		SYS_LOG_ERR("Could not register GATT service: %d", err);
		return;
	}

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

void set_meas_intervals(void)
{
	struct device *gpio = device_get_binding(SW0_GPIO_NAME);
	int button;

	gpio_pin_configure(gpio, SW0_GPIO_PIN, GPIO_DIR_IN | GPIO_PUD_PULL_UP);
	gpio_pin_write(gpio, SW0_GPIO_PIN, 1);
	gpio_pin_read(gpio, SW0_GPIO_PIN, &button);
	printf("Button: %d\n", button);
	gpio_pin_write(gpio, SW0_GPIO_PIN, 0);
	gpio_pin_configure(gpio, SW0_GPIO_PIN, GPIO_DIR_IN);
	meas_intervals = button ? &fast_meas_intervals : &slow_meas_intervals;
}

void main(void)
{
	int err;

	err = bt_enable(bt_ready_cb);
	if (err)
		SYS_LOG_ERR("Bluetooth enable failed: %d", err);

	bt_conn_cb_register(&bt_conn_callbacks);

	set_meas_intervals();

	gpio_test();

	_sys_soc_set_power_state(SYS_POWER_STATE_CPU_LPS_1);

	for (;;) {
		k_sleep(5000);
	}
}
