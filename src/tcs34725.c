#include <device.h>
#include <i2c.h>
#include <sensor.h>
#include <stdio.h>

#include "tcs34725.h"

static int tcs34725_init(struct device *dev)
{
	struct tcs34725_data *data = dev->driver_data;
	uint8_t id;
	int err;

	/* TODO make this configurable */
	data->i2c = device_get_binding(CONFIG_I2C_0_NAME);
	if (!data->i2c) {
		printf("Failed to get device binding\n");
		return -ENODEV;
	}

	err = i2c_reg_write_byte(data->i2c, TCS34725_ADDRESS, TCS34725_ENABLE,
				 TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);

	err = i2c_reg_read_byte(data->i2c, TCS34725_ADDRESS, TCS34725_ID, &id);
	if (err < 0) {
		printf("Reading tcs34725 id register failed: %d\n", err);
		return err;
	}

	printf("id: 0x%x\n", id);

	if (id != 0x44 && id != 0x4d) {
		printf("Invalid tcs34725 id: 0x%02x\n", id);
		return err;
	}

	err = i2c_reg_write_byte(data->i2c, TCS34725_ADDRESS, TCS34725_ENABLE,
				 TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
	if (err) {
		printf("Enabling tcs34725 failed\n");
		return err;
	}

	return 0;
}

static int tcs34725_sample_fetch(struct device *dev, enum sensor_channel chan)
{
	struct tcs34725_data *data = dev->driver_data;
	struct device *i2c = data->i2c;
	struct tcs34725_sample sample;
	int err;

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL);

	err = i2c_burst_read(i2c, TCS34725_ADDRESS, TCS34725_CDATA,
			     (u8_t *)&sample, sizeof(sample));
	if (err) {
		printf("Reading the color sensor failed: %d\n", err);
		return err;
	}

	data->sample = sample;

	return 0;
}

static int tcs34725_channel_get(struct device *dev, enum sensor_channel chan,
				struct sensor_value *val)
{
	struct tcs34725_data *data = dev->driver_data;
	struct device *i2c = data->i2c;
	enum tcs34725_reg reg;
	u16_t sample;
	int err;

	switch (chan) {
	case SENSOR_CHAN_LIGHT:	reg = TCS34725_CDATA; break;
	case SENSOR_CHAN_RED:	reg = TCS34725_RDATA; break;
	case SENSOR_CHAN_GREEN:	reg = TCS34725_GDATA; break;
	case SENSOR_CHAN_BLUE:	reg = TCS34725_BDATA; break;
	default: return -ENOTSUP;
	}

	err = i2c_burst_read(i2c, TCS34725_ADDRESS, reg, (u8_t *)&sample,
			     sizeof(sample));
	if (err) {
		printf("Reading the color sensor failed: %d\n", err);
		return err;
	}

	val->val1 = sample;
	val->val2 = 0;

	return 0;
}

static const struct sensor_driver_api tcs34725_driver_api = {
	.sample_fetch	= tcs34725_sample_fetch,
	.channel_get	= tcs34725_channel_get,
};

static struct tcs34725_data tcs34725_data;

DEVICE_AND_API_INIT(tcs34725, "TCS34725", &tcs34725_init,
		&tcs34725_data, NULL, POST_KERNEL,
		CONFIG_SENSOR_INIT_PRIORITY, &tcs34725_driver_api);
