/*
 * This file is part of Weather Station project <https://github.com/VeniaminGH/Weather-Station>.
 * Copyright (c) 2024 Veniamin Milevski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed WITHOUT ANY WARRANTY. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 */

#include "wst_sensor_utils.h"

#include <stdlib.h>
#include <zephyr/sys/util.h>

static int64_t shifted_q31_to_scaled_int64(q31_t q, int8_t shift, int64_t scale);

const char* wst_sensor_get_channel_name(uint16_t chan_type)
{
	switch (chan_type) {
		case SENSOR_CHAN_DIE_TEMP:
			return "Die Temperature";
		case SENSOR_CHAN_LIGHT:
			return "Illuminance";
		case SENSOR_CHAN_AMBIENT_TEMP:
			return "Ambient Temperature";
		case SENSOR_CHAN_HUMIDITY:
			return "Humidity";
		case SENSOR_CHAN_PRESS:
			return "Pressure";
		case SENSOR_CHAN_GAS_RES:
			return "Gas Resistance";
		case SENSOR_CHAN_ACCEL_XYZ:
			return "Acceleration";
		case SENSOR_CHAN_GYRO_XYZ:
			return "Angular Velocity";
	}
	return "Unknown";
}

wst_sensor_format_t wst_sensor_get_channel_format(uint16_t chan_type)
{
	switch (chan_type) {
		case SENSOR_CHAN_PROX:
			return wst_sensor_format_byte_data;
		case SENSOR_CHAN_GAUGE_CYCLE_COUNT:
			return wst_sensor_format_uint64_data;
		case SENSOR_CHAN_ACCEL_X:
		case SENSOR_CHAN_ACCEL_Y:
		case SENSOR_CHAN_ACCEL_Z:
		case SENSOR_CHAN_ACCEL_XYZ:
		case SENSOR_CHAN_GYRO_X:
		case SENSOR_CHAN_GYRO_Y:
		case SENSOR_CHAN_GYRO_Z:
		case SENSOR_CHAN_GYRO_XYZ:
		case SENSOR_CHAN_MAGN_X:
		case SENSOR_CHAN_MAGN_Y:
		case SENSOR_CHAN_MAGN_Z:
		case SENSOR_CHAN_MAGN_XYZ:
		case SENSOR_CHAN_POS_DX:
		case SENSOR_CHAN_POS_DY:
		case SENSOR_CHAN_POS_DZ:
		case SENSOR_CHAN_POS_DXYZ:
			return wst_sensor_format_3d_vector;
		default:
			return wst_sensor_format_scalar;
	}
}

void wst_q31_to_sensor_value(q31_t q, int8_t shift, struct sensor_value *val)
{
	int64_t micro_value = shifted_q31_to_scaled_int64(q, shift, 1000000LL);

	sensor_value_from_micro(val, micro_value);
}

float wst_q31_to_float(q31_t q, int8_t shift)
{
	struct sensor_value val;

	wst_q31_to_sensor_value(q, shift, &val);

	return sensor_value_to_float(&val);
}

/*
 * Copyright (c) 2023 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * shifted_q31_to_scaled_int64() implementation is taken from
 * /zephyr/subsys/sensing/sensor/phy_3d_sensor/phy_3d_sensor.c module.
 */
static int64_t shifted_q31_to_scaled_int64(q31_t q, int8_t shift, int64_t scale)
{
	int64_t scaled_value;
	int64_t shifted_value;

	shifted_value = (int64_t)q << shift;
	shifted_value = llabs(shifted_value);

	scaled_value =
		FIELD_GET(GENMASK64(31 + shift, 31), shifted_value) * scale +
		(FIELD_GET(GENMASK64(30, 0), shifted_value) * scale / BIT(31));

	if (q < 0) {
		scaled_value = -scaled_value;
	}

	return scaled_value;
}
