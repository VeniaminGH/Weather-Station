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

#include "wst_app_thread.h"
#include "wst_shared.h"
#include "wst_sensor_config.h"
#include "wst_sensor_utils.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/libc-hooks.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(wst_app_thread);

static void log_sensor_data(const wst_event_msg_t* msg)
{
	for (uint16_t i = 0; i < msg->sensor.count; i++) {

		const wst_sensor_value_t* value = &msg->sensor.values[i];

		switch (wst_sensor_get_channel_format(value->spec.chan_type)) {

			case wst_sensor_format_scalar:
				LOG_INF("%-20s for channel: type %2u, index %2u, value - %" PRIsensor_q31_data,
					wst_sensor_get_channel_name(value->spec.chan_type),
					value->spec.chan_type,
					value->spec.chan_idx,
					PRIsensor_q31_data_arg(value->data.q31_data, 0)
				);
				break;

			case wst_sensor_format_3d_vector:
				LOG_INF("%-20s for channel: type %2u, index %2u, value - %" PRIsensor_three_axis_data,
					wst_sensor_get_channel_name(value->spec.chan_type),
					value->spec.chan_type,
					value->spec.chan_idx,
					PRIsensor_three_axis_data_arg(value->data.q31_3d_data, 0)
				);
				break;

			case wst_sensor_format_occurence:
			case wst_sensor_format_byte_data:
			case wst_sensor_format_uint64_data:
			default:
				break;
		};
	}
}

static void application_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	LOG_DBG("Application thread entered");

	wst_event_msg_t* msg;

	while (1) {
		msg = k_queue_get(&shared_queue_incoming, K_FOREVER);

		if (msg == NULL) {
			LOG_ERR("no msg?");
			k_panic();
		}

		switch (msg->event)
		{
		case wst_event_sensor_data_available:
			log_sensor_data(msg);
			break;
		
		default:
			break;
		}

		// free the message
		sys_heap_free(&shared_pool, msg);
	}
}

//
// Supervisor mode setup function for APP Thread
//
void wst_app_thread_entry(void *p1, void *p2, void *p3)
{
	LOG_INF("APP thread entry");

	//
	// Grant ourselves access to kernel device objects and shared queues.
	//
	k_thread_access_grant(
		k_current_get(),
		&shared_queue_incoming,
		&shared_queue_outgoing
	);

	//
	// Switch APP thread to user mode
	//
	k_thread_user_mode_enter(application_thread, NULL, NULL, NULL);
}
