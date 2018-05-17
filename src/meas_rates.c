#include "meas_rates.h"

#if 1
/* Debug */
const struct meas_intervals fast_meas_intervals = {
	.ccs811		= 10000,
	.hts221		= 5000,
	.tcs34725	= 5000,
};
#else
const struct meas_intervals fast_meas_intervals = {
	.ccs811		= 600000,
	.hts221		= 600000,
	.tcs34725	= 600000,
};
#endif

const struct meas_intervals slow_meas_intervals = {
	.ccs811		= 10800000,
	.hts221		= 3600000,
	.tcs34725	= 3600000,
};

const struct meas_intervals *meas_intervals = &fast_meas_intervals;

