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

#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>

#define SHARED_POOL_SIZE (2048)

extern struct k_mem_partition shared_partition;

extern struct sys_heap shared_pool;
extern uint8_t shared_pool_mem[SHARED_POOL_SIZE];

extern struct k_queue shared_queue_incoming;
extern struct k_queue shared_queue_outgoing;


typedef enum wst_event {
	wst_event_lorawan_join,
	wst_event_lorawan_datarate,
	wst_event_lorawan_send,
	wst_event_lorawan_send_completed,
	wst_event_lorawan_received,
	wst_event_sensor_data_available,
} wst_event_t;

typedef struct wst_lorawan_datarate {
	uint8_t dr;
	size_t next_size;
	size_t max_size;
} wst_lorawan_datarate_t;

typedef struct wst_lorawan_send {
	size_t size;
	uint8_t payload[0];
} wst_lorawan_send_t;

typedef struct wst_lorawan_send_completed {
	int result;
} wst_lorawan_send_completed_t;

typedef struct wst_lorawan_received {
	size_t size;
	uint8_t payload[0];
} wst_lorawan_received_t;

typedef union wst_sensor_data {
	struct sensor_three_axis_data q31_3d_data;
	struct sensor_q31_data q31_data;
	struct sensor_occurrence_data occurence;
	struct sensor_byte_data byte_data;
	struct sensor_uint64_data uint64_data;
} wst_sensor_data_t;

typedef struct wst_sensor_value {
	struct sensor_chan_spec spec;
	wst_sensor_data_t data;
} wst_sensor_value_t;

typedef struct wst_event_msg {
	wst_event_t event;
	union {
		struct {
			uint16_t count;
			wst_sensor_value_t values[0];
		} sensor;
		union {
			wst_lorawan_datarate_t datarate;
			wst_lorawan_send_t send;
			wst_lorawan_send_completed_t send_completed;
			wst_lorawan_received_t received;
		} lorawan;
	};
} wst_event_msg_t;
