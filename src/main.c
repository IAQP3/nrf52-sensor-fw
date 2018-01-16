#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include <stdio.h>

#include "on_chip_temp.h"

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
		printf("Bluetooth init failed: %d\n", err);
		return;
	}

	err = on_chip_temp_init();
	if (err)
		return;

	err = bt_le_adv_start(BT_LE_ADV_CONN, bt_ad, ARRAY_SIZE(bt_ad),
			      bt_sd, ARRAY_SIZE(bt_sd));
	if (err) {
		printf("Bluetooth advertising start failed: %d\n", err);
		return;
	}
}

static void bt_connected_cb(struct bt_conn *conn, u8_t err)
{
	if (err) {
		printf("Bluetooth connect failed: %d\n", err);
		return;
	}
	printf("Bluetooth connected\n");
}

static void bt_disconnected_cb(struct bt_conn *conn, u8_t reason)
{
	printf("Bluetooth disconnected: %d\n", reason);
}

static struct bt_conn_cb bt_conn_callbacks = {
	.connected = bt_connected_cb,
	.disconnected = bt_disconnected_cb,
};

void main(void)
{
	int err;

	err = bt_enable(bt_ready_cb);
	if (err)
		printf("Bluetooth enable failed: %d\n", err);

	bt_conn_cb_register(&bt_conn_callbacks);

	printf("Hello world!\n");

	for (;;) {
		k_sleep(500);
		on_chip_temp_update();
	}
}
