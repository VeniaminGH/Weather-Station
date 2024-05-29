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

#include "wst_main_threads.h"
#include <zephyr/kernel.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wst_main);

int main(void)
{
	k_tid_t svr_thread = k_thread_create(&supervisor_thread,
		supervisor_stack,
		SUPERVISOR_STACKSIZE,
		wst_supervisor_thread_function,
		NULL,
		NULL,
		NULL,
		-1,
		K_INHERIT_PERMS,
		K_NO_WAIT);
	LOG_INF("Supervisor thread is ceated");

	k_tid_t usr_thread = k_thread_create(&user_thread,
		user_stack,
		USER_STACKSIZE,
		wst_user_thread_function,
		NULL,
		NULL,
		NULL,
		-1,
		K_USER | K_INHERIT_PERMS,
		K_NO_WAIT);
	LOG_INF("User thread is ceated");

	k_thread_join(svr_thread, K_FOREVER);
	k_thread_join(usr_thread, K_FOREVER);

	return 0;
}
