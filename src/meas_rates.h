#ifndef MEAS_RATES_H
#define MEAS_RATES_H

#include <kernel.h>

struct meas_intervals {
	s32_t ccs811;
	s32_t hts221;
	s32_t tcs34725;
};

const struct meas_intervals fast_meas_intervals;
const struct meas_intervals slow_meas_intervals;
const struct meas_intervals *meas_intervals;

#endif
