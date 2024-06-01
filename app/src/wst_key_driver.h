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
	WST_KEY_ID_0,
	WST_KEY_ID_1,
	WST_KEY_ID_2,
} wst_key_id_t;

typedef enum {
	WST_KEY_EVENT_NONE,
	WST_KEY_EVENT_KEY_UP,
	WST_KEY_EVENT_KEY_DOWN,
	WST_KEY_EVENT_KEY_SINGLE_PRESS
} wst_key_event_t;

typedef void (*key_event_handler_t)(wst_key_id_t id, wst_key_event_t event);

bool wst_key_driver_init(key_event_handler_t handler);
