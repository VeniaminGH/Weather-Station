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
#include "wst_sensor_utils.h"

#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>


#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
//#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(wst_sensor_config);

//
// Configure DT compatibility to wst,senor.yaml profile
//
#define DT_DRV_COMPAT wst_sensor

//
// Declare iodevs for sensors RTIO
//
#define WST_DT_SENSOR_CHANNEL_ARRAY_DEFINE(node_id, prop, idx)	\
	{.chan_type = DT_PROP_BY_IDX(node_id, prop, idx), .chan_idx = 0}


#define WST_DT_READ_IODEV(_inst)												\
	SENSOR_DT_READ_IODEV(														\
		_CONCAT(sensor_iodev, _inst),											\
		DT_PHANDLE(DT_DRV_INST(_inst), sensor_device),							\
		DT_INST_FOREACH_PROP_ELEM_SEP(											\
			_inst, channel_types,												\
			WST_DT_SENSOR_CHANNEL_ARRAY_DEFINE, (,))							\
);

DT_INST_FOREACH_STATUS_OKAY(WST_DT_READ_IODEV);


#define WST_DT_READ_IODEV_REFERENCE_DEFINE(_inst)								\
	_CONCAT(&sensor_iodev, _inst),

static struct rtio_iodev* iodevs[] = {
	DT_INST_FOREACH_STATUS_OKAY(WST_DT_READ_IODEV_REFERENCE_DEFINE)
};

//
// Declare sensors info
//
#define WST_DT_SENSOR_DEVICE_DEFINE(_inst)										\
	DEVICE_DT_GET(DT_PHANDLE(DT_DRV_INST(_inst), sensor_device))

#define WST_DT_SENSOR_INFO(_inst)												\
	static const wst_sensor_info_t _CONCAT(sensor, _inst) = {					\
		.sensor_device = WST_DT_SENSOR_DEVICE_DEFINE(_inst),					\
		.name = DT_NODE_FULL_NAME(DT_DRV_INST(_inst)),							\
		.friendly_name = DT_PROP(DT_DRV_INST(_inst), friendly_name),			\
		.channel_type_count = DT_PROP_LEN(DT_DRV_INST(_inst), channel_types),	\
		.channel_types = DT_PROP(DT_DRV_INST(_inst), channel_types),			\
	};

DT_INST_FOREACH_STATUS_OKAY(WST_DT_SENSOR_INFO);

#define WST_DT_SENSOR_INFO_REFERENCE_DEFINE(_inst)								\
	_CONCAT(&sensor, _inst),

static const wst_sensor_info_t* sensors[] = {
	DT_INST_FOREACH_STATUS_OKAY(WST_DT_SENSOR_INFO_REFERENCE_DEFINE)
};

//
// Declare sensors configuration
//
static const wst_sensor_config_t sensor_config = {
	.iodevs = iodevs,
	.sensors = sensors,
	.sensor_count = ARRAY_SIZE(sensors),
	.polling_period_ms = DT_PROP(DT_NODELABEL(sensor_config), polling_interval_ms)
};

static int get_sensor_count(void)
{
	return ARRAY_SIZE(sensors);
}

static void print_sensor_info(const wst_sensor_info_t* sensor)
{
	LOG_INF("Sensor '%s' found on [%s] device",
		sensor->name,
		sensor->sensor_device->name
	);

	LOG_INF("Supported channels on %s:", sensor->friendly_name);

	for (int i = 0; i < sensor->channel_type_count; i++) {
		LOG_INF("   %s", wst_sensor_get_channel_name(sensor->channel_types[i]));
	}
}

const wst_sensor_config_t* wst_sensor_get_config(void)
{
	LOG_INF("Sensor polling period: %d ms", sensor_config.polling_period_ms);

	for (int i = 0; i < get_sensor_count(); i++)
	{
		if (!device_is_ready(sensors[i]->sensor_device)) {
			LOG_ERR("device %s not ready.", sensors[i]->sensor_device->name);
			k_oops();
		} else {
			print_sensor_info(sensors[i]);
		}
	}


	return &sensor_config;
}
