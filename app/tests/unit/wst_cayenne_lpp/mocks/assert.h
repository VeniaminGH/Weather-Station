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

#include <stdbool.h>
#include <zephyr/fff.h>

/* List of fakes used by this unit tester */
#define ASSERT_FFF_FAKES_LIST(FAKE)           \
		FAKE(mock_check_if_assert_expected)   \

DECLARE_FAKE_VALUE_FUNC(bool, mock_check_if_assert_expected);

#define expect_assert()     (mock_check_if_assert_expected_fake.return_val = 1)
