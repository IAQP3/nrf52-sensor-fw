#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>

#include <nrf.h>

#include <stdio.h>
#include <misc/byteorder.h>

#define DEVICE_NAME	CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(CONFIG_BT_DEVICE_NAME) - 1)

static const struct bt_data bt_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x05, 0x18),
};

static const struct bt_data bt_sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

struct iaq_sensor {
	s16_t val;
};

struct iaq_sensor iaq_nrf_temp_sensor;

static ssize_t read_u16(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                        void *buf, u16_t len, u16_t offset)
{
        const u16_t *u16 = attr->user_data;
        u16_t value = sys_cpu_to_le16(*u16);

        return bt_gatt_attr_read(conn, attr, buf, len, offset, &value,
                                 sizeof(value));
}

static struct bt_gatt_attr bt_ess_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_TEMPERATURE, BT_GATT_PERM_READ,
			   read_u16, NULL, &iaq_nrf_temp_sensor.val),
	BT_GATT_CUD("On-Chip temperature", BT_GATT_PERM_READ),
};

static struct bt_gatt_service bt_ess_svc = BT_GATT_SERVICE(bt_ess_attrs);

static void bt_ready_cb(int err)
{
	if (err) {
		printf("Bluetooth init failed: %d\n", err);
		return;
	}

	err = bt_gatt_service_register(&bt_ess_svc);
	if (err) {
		printf("Registering GATT services failed: %d\n", err);
		return;
	}

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

static float nrf_temp_get(void)
{
	NRF_TEMP->TASKS_START = 1;
	while (!NRF_TEMP->EVENTS_DATARDY)
		;
	return NRF_TEMP->TEMP * 0.25f;
}

void main(void)
{
	int err;

	err = bt_enable(bt_ready_cb);
	if (err)
		printf("Bluetooth enable failed: %d\n", err);

	bt_conn_cb_register(&bt_conn_callbacks);

	printf("Hello world!\n");
	printf("temp: %.2f\n", nrf_temp_get());
}
