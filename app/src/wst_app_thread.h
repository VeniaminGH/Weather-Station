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

#include <zephyr/kernel.h>
#include <zephyr/app_memory/app_memdomain.h>

#define WST_APP_STACKSIZE	4096

void wst_app_thread_entry(void *p1, void *p2, void *p3);

extern struct k_mem_partition wst_app_partition;

#define WST_APP_DATA	K_APP_DMEM(wst_app_partition)
#define WST_APP_BSS		K_APP_BMEM(wst_app_partition)
