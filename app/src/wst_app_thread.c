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
#include "wst_key_driver.h"
#include "wst_led_driver.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/libc-hooks.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(wst_app_thread);

//
// Resource pool for allocations made by the kernel on behalf of system
// calls. Needed for k_queue_alloc_append()
//
K_HEAP_DEFINE(wst_app_resource_pool, 256 * 4 + 128);

//
// Define wst_app_partition for all globals.
//
K_APPMEM_PARTITION_DEFINE(wst_app_partition);

//
// Global data used by WST app. By tagging with WST_APP_BSS or WST_APP_DATA,
// we ensure that all this gets linked into wst_app_partition.
//
WST_APP_BSS const struct device *key_device;
WST_APP_BSS const struct device *led_device;

const struct device *const die_temp_sensor = DEVICE_DT_GET(DT_ALIAS(die_temp0));

#define DELAY_SEND K_MSEC(10000)

static void key_event_handler(
	const struct device *dev,
	wst_key_id_t id,
	wst_key_event_t event,
	void* context)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(context);

	LOG_INF("Key event: key_id - %d, event - %d", id, event);

	switch (id)
	{
		case WST_KEY_ID_0:
			wst_led_driver_set_state(
				led_device,
				WST_LED_ID_BLUE,
				(WST_KEY_EVENT_KEY_DOWN == event) ? true : false);
			break;

		case WST_KEY_ID_1:
			wst_led_driver_set_state(
				led_device,
				WST_LED_ID_GREEN,
				(WST_KEY_EVENT_KEY_DOWN == event) ? true : false);
			break;

		case WST_KEY_ID_2:
			wst_led_driver_set_state(
				led_device,
				WST_LED_ID_RED,
				(WST_KEY_EVENT_KEY_DOWN == event) ? true : false);
			break;

		default:
			break;
	}
}

static int print_die_temperature(const struct device *dev)
{
	struct sensor_value val;
	int ret;

	/* fetch sensor samples */
	ret = sensor_sample_fetch(dev);
	if (ret) {
		LOG_ERR("Failed to fetch sample (%d)\n", ret);
		return ret;
	}

	ret = sensor_channel_get(dev, SENSOR_CHAN_DIE_TEMP, &val);
	if (ret) {
		LOG_ERR("Failed to get data (%d)\n", ret);
		return ret;
	}

	LOG_INF("CPU Die temperature[%s]: %d°C\n", dev->name, val.val1);

	return 0;
}

static void application_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	LOG_DBG("Application thread entered");
	wst_io_msg_t* msg;

	//
	// Wait for indication that IO thread joined the LoRaWAN Network
	//
	while (1) {
		msg = k_queue_get(&shared_queue_incoming, K_FOREVER);

		if (msg == NULL) {
			LOG_ERR("no msg?");
			k_panic();
		}
		else if (wst_io_event_lorawan_joined == msg->event)
		{
			LOG_INF("Joined the Network!");
			// free the message
			sys_heap_free(&shared_pool, msg);
			break;
		}

		// free the message
		sys_heap_free(&shared_pool, msg);
	}

	//
	// Once connected, start sending messages
	//
	while (1) {
		print_die_temperature(die_temp_sensor);

		char* lorawan_msg = "helloworld!";
		size_t size = strlen(lorawan_msg);

		msg = sys_heap_alloc(&shared_pool, sizeof(wst_io_msg_t) + size);
		if (msg == NULL) {
			LOG_ERR("couldn't alloc memory from shared pool");
			k_panic();
		}
		msg->event = wst_io_event_lorawan_send;
		msg->size = size;
		memcpy(msg->payload, lorawan_msg, size);
		k_queue_alloc_append(&shared_queue_outgoing, msg);

		k_sleep(DELAY_SEND);
	}
}

//
// Supervisor mode setup function for APP Thread
//
void wst_app_thread_entry(void *p1, void *p2, void *p3)
{
	int ret;

	LOG_INF("APP thread entry");

	key_device = device_get_binding(WST_KEY_DRIVER_NAME);
	if (key_device == NULL) {
		LOG_ERR("Failed to bind key device!");
		k_oops();
	}

	led_device = device_get_binding(WST_LED_DRIVER_NAME);
	if (led_device == NULL) {
		LOG_ERR("Failed to bind led device!");
		k_oops();
	}

	if (!device_is_ready(die_temp_sensor)) {
		LOG_ERR("sensor: device %s not ready.\n", die_temp_sensor->name);
		k_oops();
	}

	//
	// Use default memory domain for our application thread and 
	// add application and shared partitions to it.
	//
	ret = k_mem_domain_add_partition(
		&k_mem_domain_default,
		&wst_app_partition
	);
	if (ret != 0) {
		LOG_ERR("Failed to add wst_app_partition to mem domain (%d)", ret);
		k_oops();
	}

	ret = k_mem_domain_add_partition(
		&k_mem_domain_default,
		&shared_partition
	);
	if (ret != 0) {
		LOG_ERR("Failed to add shared_partition to mem domain (%d)", ret);
		k_oops();
	}

	//
	// Assign app resource pool to serve for kernel-side allocations
	// for k_queue_alloc_append().
	//
	k_thread_heap_assign(k_current_get(), &wst_app_resource_pool);

	//
	// Grant ourselves access to kernel device objects and shared queues.
	//
	k_thread_access_grant(
		k_current_get(),
		key_device,
		led_device,
		die_temp_sensor,
		&shared_queue_incoming,
		&shared_queue_outgoing
	);

	//
	// Set the callback function for the key driver. This has to be
	// done from supervisor mode, as this code will run in supervisor
	// mode in system worker context.
	//
	wst_key_driver_set_handler(key_device, key_event_handler, NULL);

	//
	// Switch APP thread to user mode
	//
	k_thread_user_mode_enter(application_thread, NULL, NULL, NULL);
}
