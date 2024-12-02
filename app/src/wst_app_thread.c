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
#include "wst_events.h"
#include "wst_key_driver.h"
#include "wst_led_driver.h"
#include "wst_sensor_config.h"
#include "wst_sensor_utils.h"
#include "wst_cayenne_lpp.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
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

#if defined (CONFIG_WST_UI)
//
// Global data used by WST app. By tagging with WST_APP_BSS or WST_APP_DATA,
// we ensure that all this gets linked into wst_app_partition.
//
WST_APP_BSS const struct device *key_device;
WST_APP_BSS const struct device *led_device;
#endif

#if defined (CONFIG_WST_UI)
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
#endif

static void stream_sensor_data(const wst_event_msg_t* msg, cayenne_lpp_stream_t* stream)
{
	for (uint16_t i = 0; i < msg->sensor.count; i++) {

		struct sensor_value val;
		cayenne_lpp_value_t lpp_value;
		cayenne_lpp_result_t result = cayenne_lpp_result_success;
		const wst_sensor_value_t* value = &msg->sensor.values[i];

		if (wst_sensor_format_scalar == wst_sensor_get_channel_format(value->spec.chan_type)) {

			wst_q31_to_sensor_value(
				value->data.q31_data.readings[0].value,
				value->data.q31_data.shift,
				&val
			);

			LOG_INF("%-20s : %6d.%06d",
				wst_sensor_get_channel_name(value->spec.chan_type),
				((val.val1 < 0) || (val.val2 < 0)) ? 0 - abs(val.val1) : abs(val.val1),
				abs(val.val2)
			);
		};

		switch (value->spec.chan_type) {
			case SENSOR_CHAN_AMBIENT_TEMP:
			case SENSOR_CHAN_DIE_TEMP:
				{
				lpp_value.temperature_sensor.celsius = sensor_value_to_float(&val);

				result = cayenne_lpp_stream_write(
					stream,
					SENSOR_CHAN_DIE_TEMP == value->spec.chan_type ?
						(value->spec.chan_idx + 0x80) :
						value->spec.chan_idx,
					cayenne_lpp_type_temperature_sensor,
					&lpp_value);
				}
				break;

			case SENSOR_CHAN_LIGHT:
				{
				lpp_value.illuminance_sensor.lux = sensor_value_to_float(&val);

				result = cayenne_lpp_stream_write(
					stream,
					value->spec.chan_idx,
					cayenne_lpp_type_illuminance_sensor,
					&lpp_value);
				}
				break;

			case SENSOR_CHAN_HUMIDITY:
				{
				lpp_value.humidity_sensor.rh = sensor_value_to_float(&val);

				result = cayenne_lpp_stream_write(
					stream,
					value->spec.chan_idx,
					cayenne_lpp_type_humidity_sensor,
					&lpp_value);
				}
				break;

			case SENSOR_CHAN_PRESS:
				{
				lpp_value.barometer.hpa = sensor_value_to_float(&val);

				result = cayenne_lpp_stream_write(
					stream,
					value->spec.chan_idx,
					cayenne_lpp_type_barometer,
					&lpp_value);
				}
				break;

			default:
				break;
		};

		if (cayenne_lpp_result_error_end_of_stream == result) {
			// return and send what we have serialized
			return;
		}
	}
}

static void process_sensor_data_event(wst_event_msg_t* msg, size_t max_size)
{
	size_t stream_size = 0;
	const uint8_t* stream_buffer = NULL;

	cayenne_lpp_stream_t* stream = cayenne_lpp_stream_new(
		max_size,
		NULL
	);

	stream_sensor_data(msg, stream);

	stream_buffer = cayenne_lpp_stream_get_buffer(
		stream,
		NULL,
		&stream_size
	);

	if (stream_buffer)
	{
		wst_event_msg_t* io_msg = sys_heap_alloc(
			&events_pool,
			sizeof(wst_event_msg_t) + stream_size
		);

		if (io_msg == NULL) {
			LOG_ERR("couldn't alloc memory from shared pool");
			k_panic();
		}

		io_msg->event = wst_event_lorawan_send;
		io_msg->lorawan.send.size = stream_size;

		memcpy(
			io_msg->lorawan.send.payload,
			stream_buffer,
			stream_size
		);

		k_queue_alloc_append(&io_events_queue, io_msg);
	}
	cayenne_lpp_stream_delete(stream);
}

static void application_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	LOG_DBG("Application thread entered");

	wst_event_msg_t* msg;

	bool joined = false;
	bool busy = false;

	size_t max_size = 10;

	// Send join message to IO Thread
	msg = sys_heap_alloc(&events_pool, sizeof(wst_event_msg_t));
	if (msg == NULL) {
		LOG_ERR("couldn't alloc memory from shared pool");
		k_panic();
	}
	msg->event = wst_event_lorawan_join;
	k_queue_alloc_append(&io_events_queue, msg);


	//
	// Wait for indication that IO thread joined the LoRaWAN Network
	//
	while (1) {
		msg = k_queue_get(&app_events_queue, K_FOREVER);

		if (msg == NULL) {
			LOG_ERR("no msg?");
			k_panic();
		}

		switch (msg->event) {

		case wst_event_lorawan_datarate:
			LOG_INF("Datarate message received");
			if (!joined) {
				LOG_INF("Joined the Network!");
				joined = true;
			}
			LOG_INF("New Datarate: DR_%d, Next Paylaod %d, Max Payload %d",
				msg->lorawan.datarate.dr,
				msg->lorawan.datarate.next_size,
				msg->lorawan.datarate.max_size);
			max_size = msg->lorawan.datarate.max_size;
			break;

		case wst_event_sensor_data_available:
			LOG_INF("Data available message received");
			if (joined && max_size && !busy)
			{
				busy = true;
				process_sensor_data_event(msg, max_size);
			}
			break;

		case wst_event_lorawan_send_completed:
			LOG_INF("Send completed message received");
			busy = false;
			break;

		default:
			break;
		}

		// free the message
		sys_heap_free(&events_pool, msg);
	}
}

//
// Supervisor mode setup function for APP Thread
//
void wst_app_thread_entry(void *p1, void *p2, void *p3)
{
	int ret;

	LOG_INF("APP thread entry");

#if defined (CONFIG_WST_UI)
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
#endif

	if (wst_app_partition.size) {
		//
		// Use default memory domain for our application thread
		// add add application to it.
		//
		ret = k_mem_domain_add_partition(
			&k_mem_domain_default,
			&wst_app_partition
		);
		if (ret != 0) {
			LOG_ERR("Failed to add wst_app_partition to mem domain (%d)", ret);
			k_oops();
		}
	}

	//
	// Add shared partition for messaging
	//
	ret = k_mem_domain_add_partition(
		&k_mem_domain_default,
		&events_partition
	);
	if (ret != 0) {
		LOG_ERR("Failed to add shared_partition to mem domain (%d)", ret);
		k_oops();
	}

	//
	// And system-wide pool of memory used by libc malloc()
	// needed for wst_cayenne_lpp module
	//
	ret = k_mem_domain_add_partition(
		&k_mem_domain_default,
		&z_malloc_partition
	);
	if (ret != 0) {
		LOG_ERR("Failed to add z_malloc_partition to mem domain (%d)", ret);
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
#if defined (CONFIG_WST_UI)
		key_device,
		led_device,
#endif
		&app_events_queue,
		&io_events_queue
	);

#if defined (CONFIG_WST_UI)
	//
	// Set the callback function for the key driver. This has to be
	// done from supervisor mode, as this code will run in supervisor
	// mode in system worker context.
	//
	wst_key_driver_set_handler(key_device, key_event_handler, NULL);
#endif

	//
	// Switch APP thread to user mode
	//
	k_thread_user_mode_enter(application_thread, NULL, NULL, NULL);
}
