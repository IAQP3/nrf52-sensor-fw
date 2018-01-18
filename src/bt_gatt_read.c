#include <bluetooth/gatt.h>
#include <misc/byteorder.h>

#include "bt_gatt_read.h"


ssize_t read_u16(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		 void *buf, u16_t len, u16_t offset)
{
        const u16_t *u16 = attr->user_data;
        u16_t value = sys_cpu_to_le16(*u16);

        return bt_gatt_attr_read(conn, attr, buf, len, offset, &value,
                                 sizeof(value));
}

ssize_t read_u32(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		 void *buf, u16_t len, u16_t offset)
{
        const u32_t *u32 = attr->user_data;
        u32_t value = sys_cpu_to_le16(*u32);

        return bt_gatt_attr_read(conn, attr, buf, len, offset, &value,
                                 sizeof(value));
}

