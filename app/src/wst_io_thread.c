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
#include "wst_shared.h"

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

	//
	// We remain in supervisor mode to allow IO thread to access LoRaWAN radio.
	//
	LOG_INF("IO thread entered");

	ret = wst_lorawan_join();
	if (ret) {
		LOG_ERR("Failed to join Network (%d)!", ret);
		k_oops();
	}

	LOG_INF("Joined the Network!");

	wst_event_msg_t* msg;
	msg = sys_heap_alloc(&shared_pool, sizeof(wst_event_msg_t));
	if (msg == NULL) {
		LOG_ERR("couldn't alloc memory from shared pool");
		k_panic();
	}

	// Send joined message to Application Thread
	msg->event = wst_event_lorawan_joined;
	k_queue_alloc_append(&shared_queue_incoming, msg);

	while (1) {
		// Get LoRaWAN message from Aplication Thread
		msg = k_queue_get(&shared_queue_outgoing, K_FOREVER);
		if (msg == NULL) {
			LOG_ERR("no msg?");
			k_panic();
		}
		if (wst_event_lorawan_send == msg->event)
		{
			// send it to the network
			ret = wst_lorawan_send(msg->buffer.payload, msg->buffer.size);
			if (ret) {
				LOG_ERR("Failed to send data to the Network (%d)!", ret);
			}
		}
		// and free the message
		sys_heap_free(&shared_pool, msg);
	}
}
