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

#include <stddef.h>

typedef struct cayenne_lpp_encode_fixture {
	cayenne_lpp_stream_t* stream;
	size_t max_size;
} cayenne_lpp_encode_fixture_t;
