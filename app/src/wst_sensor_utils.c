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

#include <zephyr/drivers/sensor.h>

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
