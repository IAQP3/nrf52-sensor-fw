#ifndef HTS221_BT_H
#define HTS221_BT_H

#include <sys/types.h>

void hts221_bt_update(void);
void hts221_bt_init(void);
s16_t hts221_bt_temp;
u16_t hts221_bt_humid;

#endif
