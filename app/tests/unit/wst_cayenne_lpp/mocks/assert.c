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

/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "assert.h"

#include <zephyr/ztest.h>

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC(bool, mock_check_if_assert_expected);

void assert_print(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintk(fmt, ap);
	va_end(ap);
}

void assert_post_action(const char *file, unsigned int line)
{
	/* ztest_test_pass()/ztest_test_fail() are used to stop the execution
	 * If this is an unexpected assert (i.e. not following expect_assert())
	 * calling mock_check_if_assert_expected() will return 'false' as
	 * a default return value
	 */
	if (mock_check_if_assert_expected() == true) {
		printk("Assertion expected as part of a test case.\n");
		/* Mark the test as passed and stop execution:
		 * Needed in the passing scenario to prevent undefined behavior after hitting the
		 * assert. In real builds (non-UT), the system will be halted by the assert.
		 */
		ztest_test_pass();
	} else {
		/* Mark the test as failed and stop execution */
		ztest_test_fail();
	}
}
