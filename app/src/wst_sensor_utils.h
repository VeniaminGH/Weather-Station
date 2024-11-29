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

#pragma once

#include <zephyr/dsp/types.h>
#include <zephyr/drivers/sensor.h>

#include <stdint.h>

typedef enum wst_sensor_format {
	wst_sensor_format_occurence,
	wst_sensor_format_3d_vector,
	wst_sensor_format_scalar,
	wst_sensor_format_byte_data,
	wst_sensor_format_uint64_data
} wst_sensor_format_t;

const char* wst_sensor_get_channel_name(uint16_t chan_type);

wst_sensor_format_t wst_sensor_get_channel_format(uint16_t chan_type);

void wst_q31_to_sensor_value(q31_t q, int8_t shift, struct sensor_value *val);

float wst_q31_to_float(q31_t q, int8_t shift);
