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

#include "wst_sensor_thread.h"
#include "wst_app_thread.h"
#include "wst_shared.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// Define Sensor Thread
struct k_thread wst_sensor_thread;
K_THREAD_STACK_DEFINE(wst_sensor_stack, WST_SENSOR_STACKSIZE);

// Define App Thread
struct k_thread wst_app_thread;
K_THREAD_STACK_DEFINE(wst_app_stack, WST_APP_STACKSIZE);

int main(void)
{
	sys_heap_init(&shared_pool, shared_pool_mem, SHARED_POOL_SIZE);

	// Create App Thread
	k_tid_t app_thread = k_thread_create(
		&wst_app_thread,
		wst_app_stack,
		WST_APP_STACKSIZE,
		wst_app_thread_entry,
		NULL,
		NULL,
		NULL,
		-1,
		K_INHERIT_PERMS,
		K_NO_WAIT);
	LOG_INF("APP thread is ceated");

	// Create Sensor Thread
	k_tid_t sensor_thread = k_thread_create(
		&wst_sensor_thread,
		wst_sensor_stack,
		WST_SENSOR_STACKSIZE,
		wst_sensor_thread_entry,
		NULL,
		NULL,
		NULL,
		-1,
		K_INHERIT_PERMS,
		K_NO_WAIT);
	LOG_INF("SENSOR thread is ceated");

	k_thread_join(app_thread, K_FOREVER);
	k_thread_join(sensor_thread, K_FOREVER);

	return 0;
}
