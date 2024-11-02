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


#include "wst_io_thread.h"
#include "wst_app_thread.h"
#include "wst_shared.h"

#include <zephyr/kernel.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(wst_main);


// Define IO Thread
struct k_thread wst_io_thread;
K_THREAD_STACK_DEFINE(wst_io_stack, WST_IO_STACKSIZE);

// Define App Thread
struct k_thread wst_app_thread;;
K_THREAD_STACK_DEFINE(wst_app_stack, WST_APP_STACKSIZE);


int main(void)
{
	LOG_INF("APP partition: %p %zu", (void *)wst_app_partition.start,
		(size_t)wst_app_partition.size);

	LOG_INF("Shared partition: %p %zu", (void *)shared_partition.start,
		(size_t)shared_partition.size);

#ifdef Z_LIBC_PARTITION_EXISTS
	LOG_INF("libc partition: %p %zu", (void *)z_libc_partition.start,
		(size_t)z_libc_partition.size);
#endif
	sys_heap_init(&shared_pool, shared_pool_mem, SHARED_POOL_SIZE);

	// Create IO Thread
	k_tid_t io_thread = k_thread_create(
		&wst_io_thread,
		wst_io_stack,
		WST_IO_STACKSIZE,
		wst_io_thread_entry,
		NULL, NULL, NULL,
		-1,
		K_INHERIT_PERMS,
		K_NO_WAIT);
	LOG_INF("IO thread is ceated");

	// Create App Thread */
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

	k_thread_join(io_thread, K_FOREVER);
	k_thread_join(app_thread, K_FOREVER);

	return 0;
}
