#include <bluetooth/gatt.h>
#include <misc/byteorder.h>
#include <nrf.h>
#include <stdio.h>


static s32_t temp;

/* TODO These will be useful elsewhere once there are more sensors */
static ssize_t read_u16(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, u16_t len, u16_t offset)
{
        const u16_t *u16 = attr->user_data;
        u16_t value = sys_cpu_to_le16(*u16);

        return bt_gatt_attr_read(conn, attr, buf, len, offset, &value,
                                 sizeof(value));
}

static ssize_t read_u32(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			void *buf, u16_t len, u16_t offset)
{
        const u32_t *u32 = attr->user_data;
        u32_t value = sys_cpu_to_le16(*u32);

        return bt_gatt_attr_read(conn, attr, buf, len, offset, &value,
                                 sizeof(value));
}

static struct bt_gatt_attr on_chip_temp_bt_ess_attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY),
	BT_GATT_DESCRIPTOR(BT_UUID_TEMPERATURE, BT_GATT_PERM_READ, read_u32,
			   NULL, &temp),
	BT_GATT_CUD("On-Chip temperature", BT_GATT_PERM_READ),
};

static struct bt_gatt_service on_chip_temp_bt_ess_svc =
		BT_GATT_SERVICE(on_chip_temp_bt_ess_attrs);

s32_t on_chip_temp_get(void)
{
	NRF_TEMP->TASKS_START = 1;
	while (!NRF_TEMP->EVENTS_DATARDY)
		;
	return NRF_TEMP->TEMP * 250;
}

int on_chip_temp_init(void)
{
	int err;

	err = bt_gatt_service_register(&on_chip_temp_bt_ess_svc);
	if (err) {
		printf("Registering nRF TEMP GATT services failed: %d\n", err);
		return -1;
	}
	return 0;
}

void on_chip_temp_update(void)
{
	temp = on_chip_temp_get();
}

