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


#define CAYENNE_LPP_DECODER_MAX_BUFFER_SIZE		(10)


static void *cayenne_lpp_suite_setup(void)
{
	// allocate fixture and set buffer size
	cayenne_lpp_decode_fixture_t* fixture =
		malloc(
			sizeof(cayenne_lpp_decode_fixture_t) +
			CAYENNE_LPP_DECODER_MAX_BUFFER_SIZE
		);

	zassert_not_null(fixture, "failed to allocate cayenne_lpp_decode fixture");
	fixture->buffer_size = CAYENNE_LPP_DECODER_MAX_BUFFER_SIZE;

	return fixture;
}

static void cayenne_lpp_suite_before(void *f)
{
	// create decoding stream
	cayenne_lpp_decode_fixture_t* fixture = (cayenne_lpp_decode_fixture_t*) f;

	fixture->stream = cayenne_lpp_stream_new(
		fixture->buffer_size,
		fixture->buffer);

	zassert_not_null(fixture->stream, "cayenne_lpp_stream_new() fails");
}

static void cayenne_lpp_suite_after(void *f)
{
	// delete decoding stream
	cayenne_lpp_decode_fixture_t* fixture = (cayenne_lpp_decode_fixture_t*) f;
	cayenne_lpp_stream_delete(fixture->stream);
	fixture->stream = NULL;
}

static void cayenne_lpp_suite_teardown(void *f)
{
	// deallocate fixture
	free(f);
}


ZTEST_SUITE(
	/* SUITE_NAME */	cayenne_lpp_decode,
	/* PREDICATE */		NULL,
	/* setup_fn */		cayenne_lpp_suite_setup,
	/* before_fn */		cayenne_lpp_suite_before,
	/* after_fn */		cayenne_lpp_suite_after,
	/* teardown_fn */	cayenne_lpp_suite_teardown
);


/**
 * @brief Test Cayenne LPP stream decoder new/delete apis
 *
 * This test verifies stream new and delete apis for decoding.
 *
 */
ZTEST_F(cayenne_lpp_decode, test_new_stream_decoder)
{
	// working scenrio is covered by before/after implementation

	// here we only test zero size edge case
	cayenne_lpp_stream_t* stream = cayenne_lpp_stream_new(0, fixture->buffer);
	zassert_not_null(stream, "cayenne_lpp_stream_new() fails");
	cayenne_lpp_stream_delete(stream);
}

/**
 * @brief Test Cayenne LPP stream buffer access
 *
 * This test verifies access to decoding stream buffer.
 *
 */
ZTEST_F(cayenne_lpp_decode, test_new_stream_decoder_get_buffer)
{
	size_t buffer_size;
	size_t stream_size;
	const uint8_t* lpp_buffer;

	lpp_buffer = cayenne_lpp_stream_get_buffer(
		fixture->stream,
		&buffer_size,
		&stream_size
	);

	zassert_not_null(lpp_buffer, "cayenne_lpp_stream_get_buffer() fails");
	zassert_equal(fixture->buffer_size, buffer_size, "invalid buffer size");
	zassert_equal(fixture->buffer_size, stream_size, "invalid stream size");
	zassert_mem_equal(lpp_buffer, fixture->buffer, fixture->buffer_size, "invalid encoded data");
}
