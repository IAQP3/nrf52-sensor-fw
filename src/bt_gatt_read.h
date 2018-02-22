#ifndef BT_GATT_READ_H
#define BT_GATT_READ_H

#include <zephyr/types.h>

ssize_t read_u16(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		 void *buf, u16_t len, u16_t offset);

ssize_t read_u32(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		 void *buf, u16_t len, u16_t offset);

#endif
