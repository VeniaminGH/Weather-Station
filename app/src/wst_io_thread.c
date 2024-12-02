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
#include "wst_lorawan.h"
#include "wst_events.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/libc-hooks.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wst_io_thread);


void wst_io_thread_entry(void *p1, void *p2, void *p3)
{
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	bool joined = false;

	//
	// We remain in supervisor mode to allow IO thread to access LoRaWAN radio.
	//
	LOG_INF("IO thread entered");

	while (1) {
		// Get LoRaWAN message from Aplication Thread
		wst_event_msg_t* msg = k_queue_get(&io_events_queue, K_FOREVER);
		if (msg == NULL) {
			LOG_ERR("no msg?");
			k_panic();
		}

		switch (msg->event) {

		case wst_event_lorawan_join:
			LOG_INF("Join message received");
			ret = wst_lorawan_join();
			if (ret) {
				LOG_ERR("Failed to join Network (%d)!", ret);
				k_oops();
			} else {
				LOG_INF("Joined the Network!");
				joined = true;
			}
			break;

		case wst_event_lorawan_send:
			LOG_INF("Send message received");
			if (joined) {
				// send it to the network
				ret = wst_lorawan_send(
					msg->lorawan.send.payload,
					msg->lorawan.send.size
				);
				if (ret) {
					LOG_ERR("Failed to send data to the Network (%d)!", ret);
				}

				wst_event_msg_t* app_msg = sys_heap_alloc(
					&events_pool,
					sizeof(wst_event_msg_t)
				);
				if (app_msg == NULL) {
					LOG_ERR("couldn't alloc memory from shared pool");
					k_panic();
				}
				app_msg->event = wst_event_lorawan_send_completed;
				app_msg->lorawan.send_completed.result = ret;
				k_queue_alloc_append(&app_events_queue, app_msg);
			}
			break;

		default:
			break;
		}
		// and free the message
		sys_heap_free(&events_pool, msg);
	}
}
