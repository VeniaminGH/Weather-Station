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

#include "wst_sensor_config.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>


#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
//#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(wst_sensor_config);

#define WST_SENSOR_COUNT				(3)

static const struct device *const die_temp_sensor = DEVICE_DT_GET(DT_ALIAS(die_temp0));
SENSOR_DT_READ_IODEV(die_temp_iodev, DT_ALIAS(die_temp0),
	{SENSOR_CHAN_DIE_TEMP, 0}
);

static const struct device *const light_sensor = DEVICE_DT_GET_ONE(rohm_bh1750);
SENSOR_DT_READ_IODEV(light_iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(rohm_bh1750),
	{SENSOR_CHAN_LIGHT, 0}
);

static const struct device *const env_sensor = DEVICE_DT_GET_ONE(bosch_bme680);
SENSOR_DT_READ_IODEV(env_iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme680),
	{SENSOR_CHAN_AMBIENT_TEMP, 0},
	{SENSOR_CHAN_HUMIDITY, 0},
	{SENSOR_CHAN_PRESS, 0},
	{SENSOR_CHAN_GAS_RES, 0}
);

static struct rtio_iodev* iodevs[] = {
	&die_temp_iodev,
	&light_iodev,
	&env_iodev,
};

static const wst_sensor_config_t sensor_config = {
	.iodevs = iodevs,
	.sensor_count = WST_SENSOR_COUNT,
	.polling_period_ms = DT_PROP(DT_NODELABEL(sensor_config), polling_interval_ms)
};

const wst_sensor_config_t* wst_sensor_get_config(void)
{
	if (!device_is_ready(die_temp_sensor)) {
		LOG_ERR("device %s not ready.", die_temp_sensor->name);
		k_oops();
	} else {
		LOG_INF("%s device found", die_temp_sensor->name);
	}

	if (!device_is_ready(env_sensor)) {
		LOG_ERR("device %s not ready.", env_sensor->name);
		k_oops();
	} else {
		LOG_INF("%s device found", env_sensor->name);
	}

	if (!device_is_ready(light_sensor)) {
		LOG_ERR("device %s not ready.", light_sensor->name);
		k_oops();
	} else {
		LOG_INF("%s device found", light_sensor->name);
	}

	return &sensor_config;
}
