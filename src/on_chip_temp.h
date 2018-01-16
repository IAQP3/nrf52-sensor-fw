#ifndef ON_CHIP_TEMP_H
#define ON_CHIP_TEMP_H

#include <bluetooth/uuid.h>
#include <sys/types.h>

float on_chip_temp_get(void);
int on_chip_temp_init(void);
void on_chip_temp_update(void);

#endif
