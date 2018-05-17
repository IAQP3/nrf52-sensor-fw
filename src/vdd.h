#ifndef VDD_H
#define VDD_H

#include <device.h>
#include <misc/dlist.h>

struct vdd_rail_dev {
	bool need_init;
	sys_dlist_t node;
};

void vdd_get(void);
void vdd_put(void);

void vdd_rail_dev_register(struct vdd_rail_dev *rail_dev);
int vdd_rail_dev_call_init(struct vdd_rail_dev *rail_dev, struct device *dev);

#endif
