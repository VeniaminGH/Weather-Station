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

#pragma once

#include <stdbool.h>

typedef enum {
	WST_LED_ID_RED,
	WST_LED_ID_GREEN,
	WST_LED_ID_BLUE
} wst_led_id_t;

bool wst_led_driver_init(void);

bool wst_led_driver_set_led(wst_led_id_t id, bool state);
