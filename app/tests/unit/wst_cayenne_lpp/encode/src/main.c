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

#include <stdlib.h>


#define CAYENNE_LPP_ENCODER_MAX_BUFFER_SIZE		(10)


static void *cayenne_lpp_suite_setup(void)
{
	// allocate fixture and set max stream size
	cayenne_lpp_encode_fixture_t* fixture =
		malloc(sizeof(cayenne_lpp_encode_fixture_t));

	zassert_not_null(fixture, "failed to allocate cayenne_lpp_encode fixture");
	fixture->max_size = CAYENNE_LPP_ENCODER_MAX_BUFFER_SIZE;

	return fixture;
}

static void cayenne_lpp_suite_before(void *f)
{
	// create encoding stream
	cayenne_lpp_encode_fixture_t* fixture = (cayenne_lpp_encode_fixture_t*) f;
	fixture->stream = cayenne_lpp_stream_new(fixture->max_size, NULL);
	zassert_not_null(fixture->stream, "cayenne_lpp_stream_new() fails");
}

static void cayenne_lpp_suite_after(void *f)
{
	// delete encoding stream
	cayenne_lpp_encode_fixture_t* fixture = (cayenne_lpp_encode_fixture_t*) f;
	cayenne_lpp_stream_delete(fixture->stream);
	fixture->stream = NULL;
}

static void cayenne_lpp_suite_teardown(void *f)
{
	// deallocate fixture
	free(f);
}


ZTEST_SUITE(
	/* SUITE_NAME */	cayenne_lpp_encode,
	/* PREDICATE */		NULL,
	/* setup_fn */		cayenne_lpp_suite_setup,
	/* before_fn */		cayenne_lpp_suite_before,
	/* after_fn */		cayenne_lpp_suite_after,
	/* teardown_fn */	cayenne_lpp_suite_teardown
);


/**
 * @brief Test Cayenne LPP stream encoder new/delete apis
 *
 * This test verifies stream new and delete apis for encoding.
 *
 */
ZTEST(cayenne_lpp_encode, test_stream_encoder_new_delete)
{
	// working scenrio is covered by before/after implementation

	// here we only test zero size edge case
	cayenne_lpp_stream_t* stream = cayenne_lpp_stream_new(0, NULL);
	zassert_not_null(stream, "cayenne_lpp_stream_new() fails");
	cayenne_lpp_stream_delete(stream);

}

/**
 * @brief Test Cayenne LPP stream encoder buffer access
 *
 * This test verifies access to encoding stream buffer.
 *
 */
ZTEST_F(cayenne_lpp_encode, test_stream_encoder_get_buffer)
{
	const uint8_t* lpp_buffer;
	size_t buffer_size;
	size_t stream_size;

	lpp_buffer = cayenne_lpp_stream_get_buffer(
		fixture->stream,
		&buffer_size,
		&stream_size
	);

	zassert_not_null(lpp_buffer, "cayenne_lpp_stream_get_buffer() fails");
	zassert_equal(fixture->max_size, buffer_size, "invalid buffer size");
	zassert_equal(0, stream_size, "invalid stream size");
}

/**
 * @brief Test Cayenne LPP stream encoder buffer overflow
 *
 * This test verifies stream buffer overflow handling when encoding.
 *
 */
ZTEST_F(cayenne_lpp_encode, test_stream_encoder_buffer_overflow)
{
	cayenne_lpp_result_t result; 
	cayenne_lpp_value_t value = {.digital_input = 1};

	result = cayenne_lpp_stream_write(
		fixture->stream,
		0,
		cayenne_lpp_type_digital_input,
		&value
	);
	zassert_equal(cayenne_lpp_result_success, result);

	result = cayenne_lpp_stream_write(
		fixture->stream,
		1,
		cayenne_lpp_type_digital_input,
		&value
	);
	zassert_equal(cayenne_lpp_result_success, result);

	result = cayenne_lpp_stream_write(
		fixture->stream,
		3,
		cayenne_lpp_type_digital_input,
		&value
	);
	zassert_equal(cayenne_lpp_result_success, result);

	// expect stream buffer overflow
	result = cayenne_lpp_stream_write(
		fixture->stream,
		4,
		cayenne_lpp_type_digital_input,
		&value
	);
	zassert_equal(cayenne_lpp_result_error_overflow, result);
}
