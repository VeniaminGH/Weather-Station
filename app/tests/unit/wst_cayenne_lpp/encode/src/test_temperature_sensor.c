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
 */

#include "wst_cayenne_lpp.h"
#include "fixture.h"

#include <zephyr/ztest.h>
#include <zephyr/sys/util.h>


typedef struct test_vector_temperature_sensor {
	const uint8_t channel;
	const cayenne_lpp_value_t input;
	const uint8_t output[4];
} test_vector_temperature_sensor_t;

static const test_vector_temperature_sensor_t temp_test_vector[] = {
	{ .channel =  0, .input = {.temperature_sensor = {.celsius =    -1.0}}, .output = {0x00, 0x67, 0xff, 0xf6} },
	{ .channel =  1, .input = {.temperature_sensor = {.celsius =     1.0}}, .output = {0x01, 0x67, 0x00, 0x0a} },
	{ .channel =  2, .input = {.temperature_sensor = {.celsius =     0.0}}, .output = {0x02, 0x67, 0x00, 0x00} },
	{ .channel =  3, .input = {.temperature_sensor = {.celsius =    32.0}}, .output = {0x03, 0x67, 0x01, 0x40} },
	{ .channel =  4, .input = {.temperature_sensor = {.celsius =    32.4}}, .output = {0x04, 0x67, 0x01, 0x44} },
	{ .channel =  5, .input = {.temperature_sensor = {.celsius =    32.5}}, .output = {0x05, 0x67, 0x01, 0x45} },
	{ .channel =  6, .input = {.temperature_sensor = {.celsius =    32.6}}, .output = {0x06, 0x67, 0x01, 0x46} },
	{ .channel =  7, .input = {.temperature_sensor = {.celsius =    32.9}}, .output = {0x07, 0x67, 0x01, 0x49} },
	{ .channel =  8, .input = {.temperature_sensor = {.celsius =    33.0}}, .output = {0x08, 0x67, 0x01, 0x4a} },
	{ .channel =  9, .input = {.temperature_sensor = {.celsius =   -32.0}}, .output = {0x09, 0x67, 0xfe, 0xc0} },
	{ .channel = 10, .input = {.temperature_sensor = {.celsius =   -32.4}}, .output = {0x0a, 0x67, 0xfe, 0xbc} },
	{ .channel = 11, .input = {.temperature_sensor = {.celsius =   -32.5}}, .output = {0x0b, 0x67, 0xfe, 0xbb} },
	{ .channel = 12, .input = {.temperature_sensor = {.celsius =   -32.6}}, .output = {0x0c, 0x67, 0xfe, 0xba} },
	{ .channel = 13, .input = {.temperature_sensor = {.celsius =   -32.9}}, .output = {0x0d, 0x67, 0xfe, 0xb7} },
	{ .channel = 14, .input = {.temperature_sensor = {.celsius =   -33.0}}, .output = {0x0e, 0x67, 0xfe, 0xb6} },
	{ .channel = 15, .input = {.temperature_sensor = {.celsius =  3276.7}}, .output = {0x0f, 0x67, 0x7f, 0xff} },
	{ .channel = 16, .input = {.temperature_sensor = {.celsius = -3276.8}}, .output = {0x10, 0x67, 0x80, 0x00} },
};

/**
 * @brief Test Cayenne LPP temperature encoding
 *
 * This test verifies temperature encoding imlementation
 *
 */
ZTEST_F(cayenne_lpp_encode, test_temperature_encoding)
{
	cayenne_lpp_result_t result; 

	for (int i = 0; i < ARRAY_SIZE(temp_test_vector); i++) {

		// reset write pointer
		cayenne_lpp_stream_reset(fixture->stream);

		result = cayenne_lpp_stream_write(
			fixture->stream,
			temp_test_vector[i].channel,
			cayenne_lpp_type_temperature_sensor,
			&temp_test_vector[i].input
		);
		zassert_equal(cayenne_lpp_result_success, result, "cayenne_lpp_stream_write() fails");

		size_t buffer_size;
		size_t stream_size;

		const uint8_t* lpp_buffer = cayenne_lpp_stream_get_buffer(
			fixture->stream,
			&buffer_size,
			&stream_size
		);
		zassert_not_null(lpp_buffer, "cayenne_lpp_stream_get_buffer() fails");
		zassert_equal(fixture->max_size, buffer_size, "invalid buffer size");
		zassert_equal(sizeof(temp_test_vector[i].output), stream_size, "invalid stream size");
		zassert_mem_equal(lpp_buffer, temp_test_vector[i].output, stream_size, "invalid encoded data");
	}
}

/**
 * @brief Test Cayenne LPP temperature out of range handling
 *
 * This test verifies temperature out of range handling
 *
 */
ZTEST_F(cayenne_lpp_encode, test_temperature_out_of_range)
{
	cayenne_lpp_result_t result;
	cayenne_lpp_value_t value;

	value.temperature_sensor.celsius = 3276.8;
	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_temperature_sensor,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_out_of_range, result);

	cayenne_lpp_stream_reset(fixture->stream);

	value.temperature_sensor.celsius = -3276.9;
	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_analog_input,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_out_of_range, result);
}
