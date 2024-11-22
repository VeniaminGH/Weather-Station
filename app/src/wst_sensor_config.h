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

#include <zephyr/rtio/rtio.h>

#include <stdint.h>

typedef struct wst_sensor_info {
	const struct device* sensor_device;
	const char *name;
	const char *friendly_name;
	const int channel_type_count;
	const int32_t channel_types[];
} wst_sensor_info_t;


typedef struct wst_sensor_config {
	uint16_t sensor_count;
	uint32_t polling_period_ms;

	struct rtio_iodev** iodevs;
	const wst_sensor_info_t** sensors;
} wst_sensor_config_t;

const wst_sensor_config_t* wst_sensor_get_config(void);
