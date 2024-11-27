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


typedef struct test_vector_digital_inout {
	const uint8_t channel;
	const cayenne_lpp_value_t input;
	const uint8_t output[3];
} test_vector_digital_inout_t;

static const test_vector_digital_inout_t digital_in_test_vector[] = {
	{ .channel = 1, .input = {.digital_input =   0}, .output = {0x01, 0x00, 0x00} },
	{ .channel = 2, .input = {.digital_input = 127}, .output = {0x02, 0x00, 0x7f} },
	{ .channel = 3, .input = {.digital_input = 128}, .output = {0x03, 0x00, 0x80} },
	{ .channel = 4, .input = {.digital_input = 255}, .output = {0x04, 0x00, 0xff} },
};

static const test_vector_digital_inout_t digital_out_test_vector[] = {
	{ .channel = 1, .input = {.digital_output =   0}, .output = {0x01, 0x01, 0x00} },
	{ .channel = 2, .input = {.digital_output = 127}, .output = {0x02, 0x01, 0x7f} },
	{ .channel = 3, .input = {.digital_output = 128}, .output = {0x03, 0x01, 0x80} },
	{ .channel = 4, .input = {.digital_output = 255}, .output = {0x04, 0x01, 0xff} },
};

typedef struct test_vector_analog_inout {
	const uint8_t channel;
	const cayenne_lpp_value_t input;
	const uint8_t output[4];
} test_vector_analog_inout_t;

static const test_vector_analog_inout_t analog_in_test_vector[] = {
	{ .channel =  0, .input = {.analog_input =    -1.0}, .output = {0x00, 0x02, 0xff, 0x9c} },
	{ .channel =  1, .input = {.analog_input =     0.0}, .output = {0x01, 0x02, 0x00, 0x00} },
	{ .channel =  2, .input = {.analog_input =     1.0}, .output = {0x02, 0x02, 0x00, 0x64} },
	{ .channel =  3, .input = {.analog_input =   -55.0}, .output = {0x03, 0x02, 0xea, 0x84} },
	{ .channel =  4, .input = {.analog_input =   -55.4}, .output = {0x04, 0x02, 0xea, 0x5c} },
	{ .channel =  5, .input = {.analog_input =   -55.5}, .output = {0x05, 0x02, 0xea, 0x52} },
	{ .channel =  6, .input = {.analog_input =   -55.6}, .output = {0x06, 0x02, 0xea, 0x48} },
	{ .channel =  7, .input = {.analog_input =   -55.9}, .output = {0x07, 0x02, 0xea, 0x2a} },
	{ .channel =  8, .input = {.analog_input =   -56.0}, .output = {0x08, 0x02, 0xea, 0x20} },
	{ .channel =  9, .input = {.analog_input =    55.0}, .output = {0x09, 0x02, 0x15, 0x7c} },
	{ .channel = 10, .input = {.analog_input =    55.4}, .output = {0x0a, 0x02, 0x15, 0xa4} },
	{ .channel = 11, .input = {.analog_input =    55.5}, .output = {0x0b, 0x02, 0x15, 0xae} },
	{ .channel = 12, .input = {.analog_input =    55.6}, .output = {0x0c, 0x02, 0x15, 0xb8} },
	{ .channel = 13, .input = {.analog_input =    55.9}, .output = {0x0d, 0x02, 0x15, 0xd6} },
	{ .channel = 14, .input = {.analog_input =    56.0}, .output = {0x0e, 0x02, 0x15, 0xe0} },
	{ .channel = 15, .input = {.analog_input =  327.67}, .output = {0x0f, 0x02, 0x7f, 0xff} },
	{ .channel = 16, .input = {.analog_input = -327.68}, .output = {0x10, 0x02, 0x80, 0x00} },
};

static const test_vector_analog_inout_t analog_out_test_vector[] = {
	{ .channel =  0, .input = {.analog_output =    -1.0}, .output = {0x00, 0x03, 0xff, 0x9c} },
	{ .channel =  1, .input = {.analog_output =     0.0}, .output = {0x01, 0x03, 0x00, 0x00} },
	{ .channel =  2, .input = {.analog_output =     1.0}, .output = {0x02, 0x03, 0x00, 0x64} },
	{ .channel =  3, .input = {.analog_output =   -55.0}, .output = {0x03, 0x03, 0xea, 0x84} },
	{ .channel =  4, .input = {.analog_output =   -55.4}, .output = {0x04, 0x03, 0xea, 0x5c} },
	{ .channel =  5, .input = {.analog_output =   -55.5}, .output = {0x05, 0x03, 0xea, 0x52} },
	{ .channel =  6, .input = {.analog_output =   -55.6}, .output = {0x06, 0x03, 0xea, 0x48} },
	{ .channel =  7, .input = {.analog_output =   -55.9}, .output = {0x07, 0x03, 0xea, 0x2a} },
	{ .channel =  8, .input = {.analog_output =   -56.0}, .output = {0x08, 0x03, 0xea, 0x20} },
	{ .channel =  9, .input = {.analog_output =    55.0}, .output = {0x09, 0x03, 0x15, 0x7c} },
	{ .channel = 10, .input = {.analog_output =    55.4}, .output = {0x0a, 0x03, 0x15, 0xa4} },
	{ .channel = 11, .input = {.analog_output =    55.5}, .output = {0x0b, 0x03, 0x15, 0xae} },
	{ .channel = 12, .input = {.analog_output =    55.6}, .output = {0x0c, 0x03, 0x15, 0xb8} },
	{ .channel = 13, .input = {.analog_output =    55.9}, .output = {0x0d, 0x03, 0x15, 0xd6} },
	{ .channel = 14, .input = {.analog_output =    56.0}, .output = {0x0e, 0x03, 0x15, 0xe0} },
	{ .channel = 15, .input = {.analog_output =  327.67}, .output = {0x0f, 0x03, 0x7f, 0xff} },
	{ .channel = 16, .input = {.analog_output = -327.68}, .output = {0x10, 0x03, 0x80, 0x00} },
};

/**
 * @brief Test Cayenne LPP digital input encoding
 *
 * This test verifies digital input encoding imlementation
 *
 */
ZTEST_F(cayenne_lpp_encode, test_digital_input_encoding)
{
	cayenne_lpp_result_t result;

	for (int i = 0; i < ARRAY_SIZE(digital_in_test_vector); i++) {

		// reset write pointer
		cayenne_lpp_stream_reset(fixture->stream);

		result = cayenne_lpp_stream_write(
			fixture->stream,
			digital_in_test_vector[i].channel,
			cayenne_lpp_type_digital_input,
			&digital_in_test_vector[i].input
		);

		zassert_equal(cayenne_lpp_result_success, result);

		size_t buffer_size;
		size_t stream_size;

		const uint8_t* lpp_buffer = cayenne_lpp_stream_get_buffer(
			fixture->stream,
			&buffer_size,
			&stream_size
		);
		zassert_not_null(lpp_buffer, "cayenne_lpp_stream_get_buffer() fails");
		zassert_equal(fixture->max_size, buffer_size, "invalid buffer size");
		zassert_equal(sizeof(digital_in_test_vector[i].output), stream_size, "invalid stream size");
		zassert_mem_equal(lpp_buffer, digital_in_test_vector[i].output, stream_size, "invalid encoded data");
	}
}

/**
 * @brief Test Cayenne LPP digital output encoding
 *
 * This test verifies digital output encoding imlementation
 *
 */
ZTEST_F(cayenne_lpp_encode, test_digital_output_encoding)
{
	cayenne_lpp_result_t result;

	for (int i = 0; i < ARRAY_SIZE(digital_out_test_vector); i++) {

		// reset write pointer
		cayenne_lpp_stream_reset(fixture->stream);

		result = cayenne_lpp_stream_write(
			fixture->stream,
			digital_out_test_vector[i].channel,
			cayenne_lpp_type_digital_output,
			&digital_out_test_vector[i].input
		);

		zassert_equal(cayenne_lpp_result_success, result);

		size_t buffer_size;
		size_t stream_size;

		const uint8_t* lpp_buffer = cayenne_lpp_stream_get_buffer(
			fixture->stream,
			&buffer_size,
			&stream_size
		);
		zassert_not_null(lpp_buffer, "cayenne_lpp_stream_get_buffer() fails");
		zassert_equal(fixture->max_size, buffer_size, "invalid buffer size");
		zassert_equal(sizeof(digital_out_test_vector[i].output), stream_size, "invalid stream size");
		zassert_mem_equal(lpp_buffer, digital_out_test_vector[i].output, stream_size, "invalid encoded data");
	}
}

/**
 * @brief Test Cayenne LPP analog input encoding
 *
 * This test verifies analog input encoding imlementation
 *
 */
ZTEST_F(cayenne_lpp_encode, test_analog_input_encoding)
{
	cayenne_lpp_result_t result; 

	for (int i = 0; i < ARRAY_SIZE(analog_in_test_vector); i++) {

		// reset write pointer
		cayenne_lpp_stream_reset(fixture->stream);

		result = cayenne_lpp_stream_write(
			fixture->stream,
			analog_in_test_vector[i].channel,
			cayenne_lpp_type_analog_input,
			&analog_in_test_vector[i].input
		);

		zassert_equal(cayenne_lpp_result_success, result);

		size_t buffer_size;
		size_t stream_size;

		const uint8_t* lpp_buffer = cayenne_lpp_stream_get_buffer(
			fixture->stream,
			&buffer_size,
			&stream_size
		);
		zassert_not_null(lpp_buffer, "cayenne_lpp_stream_get_buffer() fails");
		zassert_equal(fixture->max_size, buffer_size, "invalid buffer size");
		zassert_equal(sizeof(analog_in_test_vector[i].output), stream_size, "invalid stream size");
		zassert_mem_equal(lpp_buffer, analog_in_test_vector[i].output, stream_size, "invalid encoded data");
	}
}

/**
 * @brief Test Cayenne LPP analog output encoding
 *
 * This test verifies analog output encoding imlementation
 *
 */
ZTEST_F(cayenne_lpp_encode, test_analog_output_encoding)
{
	cayenne_lpp_result_t result; 

	for (int i = 0; i < ARRAY_SIZE(analog_out_test_vector); i++) {

		// reset write pointer
		cayenne_lpp_stream_reset(fixture->stream);

		result = cayenne_lpp_stream_write(
			fixture->stream,
			analog_out_test_vector[i].channel,
			cayenne_lpp_type_analog_output,
			&analog_out_test_vector[i].input
		);

		zassert_equal(cayenne_lpp_result_success, result);

		size_t buffer_size;
		size_t stream_size;

		const uint8_t* lpp_buffer = cayenne_lpp_stream_get_buffer(
			fixture->stream,
			&buffer_size,
			&stream_size
		);
		zassert_not_null(lpp_buffer, "cayenne_lpp_stream_get_buffer() fails");
		zassert_equal(fixture->max_size, buffer_size, "invalid buffer size");
		zassert_equal(sizeof(analog_out_test_vector[i].output), stream_size, "invalid stream size");
		zassert_mem_equal(lpp_buffer, analog_out_test_vector[i].output, stream_size, "invalid encoded data");
	}
}

/**
 * @brief Test Cayenne LPP analog input/output out of range handling
 *
 * This test verifies analog input/output out of range handling
 *
 */
ZTEST_F(cayenne_lpp_encode, test_analog_inout_out_of_range)
{
	cayenne_lpp_result_t result;
	cayenne_lpp_value_t value;

	value.analog_input = 327.68;
	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_analog_input,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_out_of_range, result);

	cayenne_lpp_stream_reset(fixture->stream);

	value.analog_input = -327.69;
	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_analog_input,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_out_of_range, result);

	cayenne_lpp_stream_reset(fixture->stream);

	value.analog_output = 327.68;
	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_analog_output,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_out_of_range, result);

	cayenne_lpp_stream_reset(fixture->stream);

	value.analog_output = -327.69;
	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_analog_output,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_out_of_range, result);
}
