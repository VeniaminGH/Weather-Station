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
 *
 * This module is based on below specification:
 *
 *   https://docs.mydevices.com/docs/lorawan/cayenne-lpp
 *
 */

#include "wst_cayenne_lpp.h"

#include <zephyr/sys/__assert.h>

#include <string.h>
#include <stdlib.h>

// Record size is field size + 1 byte for Channel + 1 byte for Type
#define CAYENNE_LPP_RECORD_SIZE(size)	((size) + 2)

#define HI_BYTE(b)	((b) >> 8)
#define LO_BYTE(b)	((b) & 0xff)

struct cayenne_lpp_stream {
	size_t size;
	uint8_t wr_pos;
	uint8_t rd_pos;
	uint8_t buffer[0];
};

typedef struct cayenne_lpp_encoding {
	size_t size;
} cayenne_lpp_encoding_t;

static const cayenne_lpp_encoding_t encoding[] = {
	[cayenne_lpp_type_digital_input]		= { .size = 1 },
	[cayenne_lpp_type_digital_output]		= { .size = 1 },
	[cayenne_lpp_type_analog_input]			= { .size = 2 },
	[cayenne_lpp_type_analog_output]		= { .size = 2 },
	[cayenne_lpp_type_temperature_sensor]	= { .size = 2 },
};

static cayenne_lpp_result_t cayenne_lpp_type_check(cayenne_lpp_type_t type)
{
	switch (type)
	{
		case cayenne_lpp_type_digital_input:
		case cayenne_lpp_type_digital_output:
		case cayenne_lpp_type_analog_input:
		case cayenne_lpp_type_analog_output:
		case cayenne_lpp_type_temperature_sensor:
			break;
		case cayenne_lpp_type_generic_sensor:
		case cayenne_lpp_type_illuminance_sensor:
		case cayenne_lpp_type_presence_sensor:
		case cayenne_lpp_type_humiditiy_sensor:
		case cayenne_lpp_type_power_measurement:
		case cayenne_lpp_type_actuation:
		case cayenne_lpp_type_set_point:
		case cayenne_lpp_type_load_control:
		case cayenne_lpp_type_light_control:
		case cayenne_lpp_type_power_control:
		case cayenne_lpp_type_accelerometer:
		case cayenne_lpp_type_magnometer:
		case cayenne_lpp_type_barometer:
		case cayenne_lpp_type_voltage:
		case cayenne_lpp_type_current:
		case cayenne_lpp_type_frequency:
		case cayenne_lpp_type_depth:
		case cayenne_lpp_type_percentage:
		case cayenne_lpp_type_altitude:
		case cayenne_lpp_type_load:
		case cayenne_lpp_type_pressure:
		case cayenne_lpp_type_loudness:
		case cayenne_lpp_type_concentration:
		case cayenne_lpp_type_acidity:
		case cayenne_lpp_type_conductivity:
		case cayenne_lpp_type_power:
		case cayenne_lpp_type_power_factor:
		case cayenne_lpp_type_distance:
		case cayenne_lpp_type_energy:
		case cayenne_lpp_type_direction:
		case cayenne_lpp_type_time:
		case cayenne_lpp_type_gyrometer:
		case cayenne_lpp_type_color:
		case cayenne_lpp_type_gps_location:
		case cayenne_lpp_type_positioner:
		case cayenne_lpp_type_buzzer:
		case cayenne_lpp_type_audio_clip:
		case cayenne_lpp_type_timer:
		case cayenne_lpp_type_addr_text_display:
		case cayenne_lpp_type_onoff_switch:
		case cayenne_lpp_type_level_control:
		case cayenne_lpp_type_updown_control:
		case cayenne_lpp_type_multi_axis_control:
		case cayenne_lpp_type_rate:
		case cayenne_lpp_type_push_button:
		case cayenne_lpp_type_multistate_selector:
			return cayenne_lpp_result_error_not_implemented;
		default:
			return cayenne_lpp_result_error_unknown_type;
	}

	return cayenne_lpp_result_success;
}

cayenne_lpp_stream_t* cayenne_lpp_stream_new(size_t size, uint8_t* buffer)
{
	cayenne_lpp_stream_t* stream = malloc(sizeof(cayenne_lpp_stream_t) + size);
	__ASSERT(stream, "Failed to allocate cayenne lpp stream context!");

	stream->size = size;
	stream->wr_pos = 0;
	stream->rd_pos = 0;

	if (buffer)
	{
		memcpy(stream->buffer, buffer, size);
		// move wr_pos to the end
		stream->wr_pos = size;
	}
	return stream;
}

void cayenne_lpp_stream_delete(cayenne_lpp_stream_t* stream)
{
	__ASSERT_NO_MSG(stream);
	free(stream);
}

cayenne_lpp_result_t cayenne_lpp_stream_write(
	cayenne_lpp_stream_t* stream,
	uint8_t channel,
	cayenne_lpp_type_t type,
	const cayenne_lpp_value_t* value)
{
	__ASSERT_NO_MSG(stream);
	__ASSERT_NO_MSG(value);

	// check supported cayenne type
	cayenne_lpp_result_t result = cayenne_lpp_type_check(type);
	if (cayenne_lpp_result_success != result) {
		return result;
	}

	// check available stream buffer space
	if (CAYENNE_LPP_RECORD_SIZE(encoding[type].size) >
		(stream->size - stream->wr_pos)) {
		return cayenne_lpp_result_error_overflow;
	}

	switch (type)
	{
		case cayenne_lpp_type_digital_input:
		case cayenne_lpp_type_digital_output:
			{
			stream->buffer[stream->wr_pos++] = channel;
			stream->buffer[stream->wr_pos++] = (uint8_t) type;
			stream->buffer[stream->wr_pos++] = value->digital_input;
			}
			break;

		case cayenne_lpp_type_analog_input:
		case cayenne_lpp_type_analog_output:
			{
			int16_t val = (int16_t) (value->analog_input * 100);
			stream->buffer[stream->wr_pos++] = channel;
			stream->buffer[stream->wr_pos++] = (uint8_t) type;
			stream->buffer[stream->wr_pos++] = HI_BYTE(val);
			stream->buffer[stream->wr_pos++] = LO_BYTE(val);
			}
			break;

		case cayenne_lpp_type_temperature_sensor:
			{
			int16_t val = (int16_t) (value->temperature_sensor.celsius * 10);
			stream->buffer[stream->wr_pos++] = channel;
			stream->buffer[stream->wr_pos++] = (uint8_t) type;
			stream->buffer[stream->wr_pos++] = HI_BYTE(val);
			stream->buffer[stream->wr_pos++] = LO_BYTE(val);
			}
			break;

		default:
			return cayenne_lpp_result_error_unknown_type;
	}

	return cayenne_lpp_result_success;
}

cayenne_lpp_result_t cayenne_lpp_stream_read(
	cayenne_lpp_stream_t* stream,
	uint8_t* channel,
	cayenne_lpp_type_t* type,
	cayenne_lpp_value_t* value)
{
	__ASSERT_NO_MSG(stream);
	return cayenne_lpp_result_error_not_implemented;
}

const uint8_t* cayenne_lpp_stream_get_buffer(
	cayenne_lpp_stream_t* stream,
	size_t* buffer_size,
	size_t* stream_size)
{
	__ASSERT_NO_MSG(stream);
	if (buffer_size) {
		*buffer_size = stream->size;
	}
	if (stream_size) {
		*stream_size = stream->wr_pos;
	}
	return stream->buffer;
}

size_t cayenne_lpp_stream_get_free_space(
	cayenne_lpp_stream_t* stream)
{
	__ASSERT_NO_MSG(stream);
	return stream->size - stream->wr_pos;
}


void cayenne_lpp_stream_reset(
	cayenne_lpp_stream_t* stream)
{
	__ASSERT_NO_MSG(stream);
	stream->wr_pos = 0;
	stream->rd_pos = 0;
}
