#ifndef TCS34725_BT_H
#define TCS34725_BT_H

#define UUID_RED	BT_UUID_DECLARE_16(0x0203)
#define UUID_GREEN	BT_UUID_DECLARE_16(0x0204)
#define UUID_BLUE	BT_UUID_DECLARE_16(0x0205)
#define UUID_LIGHT	BT_UUID_DECLARE_16(0x0206)

void tcs34725_bt_init(void);
void tcs34725_bt_update(void);

u16_t tcs34725_bt_r;
u16_t tcs34725_bt_g;
u16_t tcs34725_bt_b;
u16_t tcs34725_bt_l;

#endif
