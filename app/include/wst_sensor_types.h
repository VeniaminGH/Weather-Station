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

//
// WST channel types must match sensor channel enum values defined in sensor.h
//
#define WST_CHANNEL_TYPE_ACCEL_XYZ			(3)		// SENSOR_CHAN_ACCEL_XYZ
#define WST_CHANNEL_TYPE_GYRO_XYZ			(7)		// SENSOR_CHAN_GYRO_XYZ
#define WST_CHANNEL_TYPE_DIE_TEMP			(12)	// SENSOR_CHAN_DIE_TEMP
#define WST_CHANNEL_TYPE_AMBIENT_TEMP		(13)	// SENSOR_CHAN_AMBIENT_TEMP
#define WST_CHANNEL_TYPE_PRESS				(14)	// SENSOR_CHAN_PRESS
#define WST_CHANNEL_TYPE_HUMIDITY			(16)	// SENSOR_CHAN_HUMIDITY
#define WST_CHANNEL_TYPE_LIGHT				(17)	// SENSOR_CHAN_LIGHT


//
// Skip below by Devicetree generator
//
#if CONFIG_SENSOR

#include <zephyr/drivers/sensor.h>

_Static_assert(
	(WST_CHANNEL_TYPE_ACCEL_XYZ		== SENSOR_CHAN_ACCEL_XYZ) &&
	(WST_CHANNEL_TYPE_GYRO_XYZ		== SENSOR_CHAN_GYRO_XYZ) &&
	(WST_CHANNEL_TYPE_DIE_TEMP		== SENSOR_CHAN_DIE_TEMP) &&
	(WST_CHANNEL_TYPE_AMBIENT_TEMP	== SENSOR_CHAN_AMBIENT_TEMP) &&
	(WST_CHANNEL_TYPE_PRESS			== SENSOR_CHAN_PRESS) &&
	(WST_CHANNEL_TYPE_HUMIDITY		== SENSOR_CHAN_HUMIDITY) &&
	(WST_CHANNEL_TYPE_LIGHT			== SENSOR_CHAN_LIGHT),
	"WST channel type defines and Sensor channel enums are not matching!");

#endif
