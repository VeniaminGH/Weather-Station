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

#define WST_SENSOR_COUNT				(1)
#define WST_SENSOR_POLLING_INTERVAL_MS	(1000)

static const struct device *const acc_sensor = DEVICE_DT_GET_ONE(bosch_bmi160);
SENSOR_DT_READ_IODEV(acc_iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bmi160),
	{SENSOR_CHAN_DIE_TEMP, 0},
	{SENSOR_CHAN_ACCEL_XYZ, 0},
	{SENSOR_CHAN_GYRO_XYZ, 0}
);

static struct rtio_iodev* iodevs[] = {
	&acc_iodev,
};

static const wst_sensor_config_t sensor_config = {
	.iodevs = iodevs,
	.sensor_count = WST_SENSOR_COUNT,
	.polling_period_ms = WST_SENSOR_POLLING_INTERVAL_MS
};

const wst_sensor_config_t* wst_sensor_get_config(void)
{
	if (!device_is_ready(acc_sensor)) {
		LOG_ERR("device %s not ready.", acc_sensor->name);
		k_oops();
	} else {
		LOG_INF("%s device found", acc_sensor->name);
	}

	return &sensor_config;
}
