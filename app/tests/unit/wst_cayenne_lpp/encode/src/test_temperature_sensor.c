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

/**
 * @brief Test Cayenne LPP temperature encoding
 *
 * This test verifies temperature encoding imlementation
 *
 */
ZTEST_F(cayenne_lpp_encode, test_temperature_encoding)
{
	cayenne_lpp_result_t result; 
	const cayenne_lpp_value_t value = {.temperature_sensor.celsius = 32.5};
	const uint8_t expected_buffer[] = {0x01, 0x67, 0x01, 0x45};

	result = cayenne_lpp_stream_write(
		fixture->stream,
		1,
		cayenne_lpp_type_temperature_sensor,
		&value
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
	zassert_equal(sizeof(expected_buffer), stream_size, "invalid stream size");
	zassert_mem_equal(lpp_buffer, expected_buffer, sizeof(expected_buffer), "invalid encoded data");
}
