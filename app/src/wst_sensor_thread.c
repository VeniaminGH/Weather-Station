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

#include "wst_sensor_thread.h"
#include "wst_shared.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/sys/slist.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>


#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
//#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(wst_sensor_thread);

#define DELAY_SENSOR_INTERVAL	K_MSEC(5000)

#define NUM_SENSORS 3
#define NUM_SENSOR_CHANNELS 6

static const struct device *const die_temp_sensor = DEVICE_DT_GET(DT_ALIAS(die_temp0));
SENSOR_DT_READ_IODEV(die_temp_iodev, DT_ALIAS(die_temp0),
	{SENSOR_CHAN_DIE_TEMP, 0}
);

static const struct device *const light_sensor = DEVICE_DT_GET_ONE(rohm_bh1750);
SENSOR_DT_READ_IODEV(light_iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(rohm_bh1750),
	{SENSOR_CHAN_LIGHT, 0}
);

static const struct device *const env_sensor = DEVICE_DT_GET_ONE(bosch_bme680);
SENSOR_DT_READ_IODEV(env_iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme680),
	{SENSOR_CHAN_AMBIENT_TEMP, 0},
	{SENSOR_CHAN_HUMIDITY, 0},
	{SENSOR_CHAN_PRESS, 0},
	{SENSOR_CHAN_GAS_RES, 0}
);

struct rtio_iodev* iodevs[] = {
	&die_temp_iodev,
	&light_iodev,
	&env_iodev,
};

RTIO_DEFINE_WITH_MEMPOOL(
	temp_ctx,
	8,
	8,
	NUM_SENSOR_CHANNELS,
	64,
	sizeof(void *)
);

typedef struct wst_sensor_data {
	sys_snode_t node;
	struct sensor_chan_spec spec;
	struct sensor_q31_data data;
} wst_sensor_data_t;


static wst_sensor_data_t* add_sensor_data_node(
	sys_slist_t* list,
	struct sensor_chan_spec spec,
	struct sensor_q31_data* data)
{
	wst_sensor_data_t* sensor_data = malloc(sizeof(wst_sensor_data_t));
	sensor_data->node.next = NULL;
	sensor_data->spec = spec;
	sensor_data->data = *data;

	sys_slist_append(list, &sensor_data->node);
	return sensor_data;
}

static void free_sensor_data_node(wst_sensor_data_t* node)
{
	free(node);
}

static int decode_sensor_data(
	sys_slist_t* values,
	const struct sensor_read_config* sensor_config,
	uint8_t *buf
)
{
	const struct sensor_decoder_api *decoder;
	struct sensor_q31_data data;
	uint16_t count = 0;

	int rc = sensor_get_decoder(sensor_config->sensor, &decoder);
	if (rc != 0) {
		LOG_ERR("sensor_get_decoder failed %d", rc);
		return rc;
	}

	for (size_t i = 0; i < sensor_config->count; i++) {

		// Frame iterators, one per channel we are decoding
		uint32_t fits = 0;

		// Try to decode
		rc = decoder->decode(
			buf,
			sensor_config->channels[i],
			&fits,
			1,
			&data
		);

		if (rc < 0) {
			// In case of failure, fail gracefully,
			// and try next channel, if any.
			LOG_WRN("sensor decoding failed %d", rc);
		} else {
			// Add sensor channel data
			LOG_DBG("%s for %s channel 0, result - %d, value - %" PRIsensor_q31_data,
				wst_get_sensor_channel_name(sensor_config->channels[i].chan_type),
				sensor_config->sensor->name,
				rc,
				PRIsensor_q31_data_arg(data, 0)
			);

			add_sensor_data_node(
				values,
				sensor_config->channels[i],
				&data);

			count++;
		}
	}
	return (int) count;
}

static bool wst_sensor_get_data(sys_slist_t* values, uint16_t* count)
{
	int rc;
	struct rtio_cqe *cqe;
	uint8_t *buf;
	uint32_t buf_len;

	*count = 0;

	// Non-Blocking read for each sensor
	for (int i = 0; i < NUM_SENSORS; i++) {
		rc = sensor_read_async_mempool(iodevs[i], &temp_ctx, iodevs[i]);

		if (rc != 0) {
			LOG_ERR("sensor_read() failed %d", rc);
			return false;
		}
	}

	// Wait for read completions
	for (int i = 0; i < NUM_SENSORS; i++) {
		cqe = rtio_cqe_consume_block(&temp_ctx);

		if (cqe->result != 0) {
			LOG_ERR("async read failed %d", cqe->result);
			return false;
		}

		// Get the associated mempool buffer with the completion
		rc = rtio_cqe_get_mempool_buffer(&temp_ctx, cqe, &buf, &buf_len);

		if (rc != 0) {
			LOG_ERR("get mempool buffer failed %d", rc);
			return false;
		}

		const struct sensor_read_config* sensor_config =
			(const struct sensor_read_config *)
				((struct rtio_iodev *)cqe->userdata)->data;

		LOG_DBG("sensor_read_config: count - %u", sensor_config->count);

		// Done with the completion event, release it
		rtio_cqe_release(&temp_ctx, cqe);

		rc = decode_sensor_data(values, sensor_config, buf);
		if (rc <= 0) {
			LOG_ERR("decode_sensor_data failed %d", rc);
			return false;
		} else {
			(*count) += (uint16_t) rc;
		}

		// Done with the buffer, release it
		rtio_release_buffer(&temp_ctx, buf, buf_len);
	}

	return true;
}

void wst_sensor_thread_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	//
	// We remain in supervisor mode to allow SENSOR thread to access sensors.
	//
	LOG_INF("SENSOR thread entered");

	if (!device_is_ready(die_temp_sensor)) {
		LOG_ERR("sensor: device %s not ready.\n", die_temp_sensor->name);
		k_oops();
	}
	if (!device_is_ready(env_sensor)) {
		LOG_ERR("sensor: device %s not ready.\n", env_sensor->name);
		k_oops();
	}
	if (!device_is_ready(light_sensor)) {
		LOG_ERR("sensor: device %s not ready.\n", light_sensor->name);
		k_oops();
	}

	while (1) {
		uint16_t count = 0;
		sys_slist_t values;
		
		sys_slist_init(&values);

		// Obtain sensor data
		if (wst_sensor_get_data(&values, &count)) {

			LOG_DBG("Obtained %u sensor values", count);

			// Allocate sensor message to Applicaion thread
			wst_event_msg_t* msg = sys_heap_alloc(&shared_pool, sizeof(wst_event_msg_t) + sizeof(wst_sensor_value_t)*count);
			if (msg == NULL) {
				LOG_ERR("couldn't alloc memory from shared pool");
				k_panic();
			}

			// Initialize sensor message
			msg->event = wst_event_sensor_data_available;
			msg->sensor.count = count;

			sys_snode_t *curr, *next = NULL;
			wst_sensor_value_t* value = msg->sensor.values;

			// Iterate through sensor values list
			SYS_SLIST_FOR_EACH_NODE_SAFE(&values, curr, next) {

				// Remove sensor data node from the list
				wst_sensor_data_t* node = (wst_sensor_data_t*) sys_slist_get(&values);
				__ASSERT_NO_MSG(node);
				LOG_DBG("Added sensor type %u channel %u",
					node->spec.chan_type,
					node->spec.chan_idx
				);
				// Copy sensor data
				value->spec = node->spec;
				value->data = node->data;
				value++;
				free_sensor_data_node(node);
			}

			// Send sensor message to Application thread
			k_queue_alloc_append(&shared_queue_incoming, msg);
		}

		k_sleep(DELAY_SENSOR_INTERVAL);
	}
}

const char* wst_get_sensor_channel_name(uint16_t chan_type)
{
	switch(chan_type) {
		case SENSOR_CHAN_DIE_TEMP:
			return "Die Temperature";
		case SENSOR_CHAN_LIGHT:
			return "Illuminance";
		case SENSOR_CHAN_AMBIENT_TEMP:
			return "Ambient Temperature";
		case SENSOR_CHAN_HUMIDITY:
			return "Humidity";
		case SENSOR_CHAN_PRESS:
			return "Pressure";
		case SENSOR_CHAN_GAS_RES:
			return "Gas Resistance";
	}
	return "Unknown";
}
