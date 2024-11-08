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
#include <zephyr/sys/sys_heap.h>

#define SHARED_POOL_SIZE (2048)

extern struct k_mem_partition shared_partition;

extern struct sys_heap shared_pool;
extern uint8_t shared_pool_mem[SHARED_POOL_SIZE];

extern struct k_queue shared_queue_incoming;
extern struct k_queue shared_queue_outgoing;


typedef enum wst_io_event {
	wst_io_event_lorawan_joined,
	wst_io_event_lorawan_send,
	wst_io_event_lorawan_send_completed,
	wst_io_event_lorawan_received
} wst_io_event_t;


typedef struct wst_io_msg {
	wst_io_event_t event;
	size_t size;
	uint8_t payload[0];
} wst_io_msg_t;