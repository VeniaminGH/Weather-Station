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


typedef struct test_vector_barometer {
	const uint8_t channel;
	const cayenne_lpp_value_t input;
	const uint8_t output[4];
} test_vector_barometer_t;

static const test_vector_barometer_t barometer_test_vector[] = {
	{ .channel = 0, .input = {.barometer = {.hpa =    0.0}}, .output = {0x00, 0x73, 0x00, 0x00} },
	{ .channel = 1, .input = {.barometer = {.hpa =  100.0}}, .output = {0x01, 0x73, 0x03, 0xe8} },
	{ .channel = 2, .input = {.barometer = {.hpa =  100.4}}, .output = {0x02, 0x73, 0x03, 0xec} },
	{ .channel = 3, .input = {.barometer = {.hpa =  100.5}}, .output = {0x03, 0x73, 0x03, 0xed} },
	{ .channel = 4, .input = {.barometer = {.hpa =  100.6}}, .output = {0x04, 0x73, 0x03, 0xee} },
	{ .channel = 5, .input = {.barometer = {.hpa =  100.9}}, .output = {0x05, 0x73, 0x03, 0xf1} },
	{ .channel = 6, .input = {.barometer = {.hpa =  101.0}}, .output = {0x06, 0x73, 0x03, 0xf2} },
	{ .channel = 7, .input = {.barometer = {.hpa = 6553.5}}, .output = {0x07, 0x73, 0xff, 0xff} },
};

/**
 * @brief Test Cayenne LPP barometer encoding
 *
 * This test verifies barometer encoding imlementation
 *
 */
ZTEST_F(cayenne_lpp_encode, test_barometer_encoding)
{
	cayenne_lpp_result_t result; 

	for (int i = 0; i < ARRAY_SIZE(barometer_test_vector); i++) {

		// reset write pointer
		cayenne_lpp_stream_reset(fixture->stream);

		result = cayenne_lpp_stream_write(
			fixture->stream,
			barometer_test_vector[i].channel,
			cayenne_lpp_type_barometer,
			&barometer_test_vector[i].input
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
		zassert_equal(sizeof(barometer_test_vector[i].output), stream_size, "invalid stream size");
		zassert_mem_equal(lpp_buffer, barometer_test_vector[i].output, stream_size, "invalid encoded data");
	}
}

/**
 * @brief Test Cayenne LPP barometer out of range handling
 *
 * This test verifies barometer out of range handling
 *
 */
ZTEST_F(cayenne_lpp_encode, test_barometer_out_of_range)
{
	cayenne_lpp_result_t result;
	cayenne_lpp_value_t value;

	value.barometer.hpa = 6553.551;
	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_barometer,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_out_of_range, result);

	cayenne_lpp_stream_reset(fixture->stream);

	value.barometer.hpa = -0.05;
	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_barometer,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_out_of_range, result);
}
