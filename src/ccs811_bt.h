#ifndef CCS811_BT_H
#define CCS811_BT_H

#define UUID_PRIMARY	BT_UUID_DECLARE_16(0x0200)
#define UUID_CO2	BT_UUID_DECLARE_16(0x0201)
#define UUID_VOC	BT_UUID_DECLARE_16(0x0202)

#define CCS811_BT_MEAS_INTERVAL 5000

void ccs811_bt_update(void);
void ccs811_bt_init(void);
u16_t ccs811_bt_co2; /* In ppm */
u16_t ccs811_bt_voc; /* In ppb */

#endif
