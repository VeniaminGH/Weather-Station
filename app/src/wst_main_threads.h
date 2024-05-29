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

#ifndef MAIN_THREADS_H
#define MAIN_THREADS_H

#include <zephyr/kernel.h>

#define SUPERVISOR_STACKSIZE	4096
struct k_thread supervisor_thread;
K_THREAD_STACK_DEFINE(supervisor_stack, SUPERVISOR_STACKSIZE);
void wst_user_thread_function(void *p1, void *p2, void *p3);

#define USER_STACKSIZE			2048
struct k_thread user_thread;
K_THREAD_STACK_DEFINE(user_stack, USER_STACKSIZE);
void wst_supervisor_thread_function(void *p1, void *p2, void *p3);

#endif /* MAIN_THREADS_H */
