#ifndef ON_CHIP_TEMP_H
#define ON_CHIP_TEMP_H

#include <sys/types.h>

#define ON_CHIP_TEMP_MEAS_INTERVAL 1000

/* Temperature in Centicelsius */
s16_t on_chip_temp_get(void);
void on_chip_temp_update(void);

s16_t on_chip_temp;

#endif
