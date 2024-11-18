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
#include "wst_sensor_config.h"
#include "wst_sensor_utils.h"
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

#define WST_SENSOR_RTIO_SQE_NUM		(8)		// Number of the sensing RTIO SQE
#define WST_SENSOR_RTIO_CQE_NUM		(8)		// Number of the sensing RTIO SQE
#define WST_SENSOR_RTIO_BLOCK_SIZE	(64)	// Block size of the RTIO context
#define WST_SENSOR_RTIO_BLOCK_COUNT	(8)		// Number of memory blocks of the RTIO context

RTIO_DEFINE_WITH_MEMPOOL(
	rtio_ctx,
	WST_SENSOR_RTIO_SQE_NUM,
	WST_SENSOR_RTIO_CQE_NUM,
	WST_SENSOR_RTIO_BLOCK_COUNT,
	WST_SENSOR_RTIO_BLOCK_SIZE,
	sizeof(void *)
);

typedef struct wst_sensor_node_data {
	sys_snode_t node;
	struct sensor_chan_spec spec;
	wst_sensor_data_t data;
} wst_sensor_node_data_t;


static wst_sensor_node_data_t* add_sensor_data_node(
	sys_slist_t* list,
	struct sensor_chan_spec spec,
	wst_sensor_data_t* data)
{
	wst_sensor_node_data_t* sensor_data = malloc(sizeof(wst_sensor_node_data_t));
	sensor_data->node.next = NULL;
	sensor_data->spec = spec;
	sensor_data->data = *data;

	sys_slist_append(list, &sensor_data->node);
	return sensor_data;
}

static void free_sensor_data_node(wst_sensor_node_data_t* node)
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
	wst_sensor_data_t data;
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
			switch (wst_sensor_get_channel_format(sensor_config->channels[i].chan_type)) {

				case wst_sensor_format_scalar:
					LOG_DBG("%s for %s channel 0, result - %d, value - %" PRIsensor_q31_data,
						wst_sensor_get_channel_name(sensor_config->channels[i].chan_type),
						sensor_config->sensor->name,
						rc,
						PRIsensor_q31_data_arg(data.q31_data, 0)
					);
					break;

				case wst_sensor_format_3d_vector:
					LOG_DBG("%s for %s channel 0, result - %d, value - %" PRIsensor_three_axis_data,
						wst_sensor_get_channel_name(sensor_config->channels[i].chan_type),
						sensor_config->sensor->name,
						rc,
						PRIsensor_three_axis_data_arg(data.q31_3d_data, 0)
					);
					break;

				case wst_sensor_format_occurence:
				case wst_sensor_format_byte_data:
				case wst_sensor_format_uint64_data:
				default:
					break;
			};

			add_sensor_data_node(
				values,
				sensor_config->channels[i],
				&data);

			count++;
		}
	}
	return (int) count;
}

static uint16_t wst_sensor_get_data(const wst_sensor_config_t* config, sys_slist_t* values)
{
	int rc;
	struct rtio_cqe *cqe;
	uint8_t *buf;
	uint32_t buf_len;

	uint16_t count = 0;

	// Non-Blocking read for each sensor
	for (int i = 0; i < config->sensor_count; i++) {
		rc = sensor_read_async_mempool(config->iodevs[i], &rtio_ctx, config->iodevs[i]);

		if (rc != 0) {
			LOG_ERR("sensor_read() failed %d", rc);
			return count;
		}
	}

	// Wait for read completions
	for (int i = 0; i < config->sensor_count; i++) {
		cqe = rtio_cqe_consume_block(&rtio_ctx);

		if (cqe->result != 0) {
			LOG_ERR("async read failed %d", cqe->result);
			return count;
		}

		// Get the associated mempool buffer with the completion
		rc = rtio_cqe_get_mempool_buffer(&rtio_ctx, cqe, &buf, &buf_len);

		if (rc != 0) {
			LOG_ERR("get mempool buffer failed %d", rc);
			return count;
		}

		const struct sensor_read_config* read_config =
			(const struct sensor_read_config *)
				((struct rtio_iodev *)cqe->userdata)->data;

		LOG_DBG("sensor_read_config: count - %u", read_config->count);

		// Done with the completion event, release it
		rtio_cqe_release(&rtio_ctx, cqe);

		rc = decode_sensor_data(values, read_config, buf);
		if (rc <= 0) {
			LOG_ERR("decode_sensor_data failed %d", rc);
			return count;
		} else {
			count += (uint16_t) rc;
		}

		// Done with the buffer, release it
		rtio_release_buffer(&rtio_ctx, buf, buf_len);
	}

	return count;
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

	const wst_sensor_config_t* sensor_config = wst_sensor_get_config();
	if (!sensor_config) {
		LOG_ERR("Failed to get sensor configuration!");
		k_panic();
	}

	while (1) {
		uint16_t count = 0;
		sys_slist_t values_l;
		
		sys_slist_init(&values_l);

		// Obtain sensor data
		count = wst_sensor_get_data(sensor_config, &values_l);
		if (count) {

			LOG_DBG("Obtained %u sensor values", count);

			// Allocate sensor message to Applicaion thread
			wst_event_msg_t* msg = sys_heap_alloc(&shared_pool,
				sizeof(wst_event_msg_t) + sizeof(wst_sensor_value_t) * count);

			if (!msg) {
				LOG_ERR("couldn't alloc memory from shared pool");
				k_panic();
			}

			// Initialize sensor message
			msg->event = wst_event_sensor_data_available;
			msg->sensor.count = count;

			sys_snode_t *curr, *next = NULL;
			wst_sensor_value_t* sensor_value = msg->sensor.values;

			// Iterate through sensor values list
			SYS_SLIST_FOR_EACH_NODE_SAFE(&values_l, curr, next) {

				// Remove sensor data node from the list
				wst_sensor_node_data_t* node = (wst_sensor_node_data_t*) sys_slist_get(&values_l);
				__ASSERT_NO_MSG(node);
				LOG_DBG("Added sensor type %u channel %u",
					node->spec.chan_type,
					node->spec.chan_idx
				);
				// Copy sensor data
				sensor_value->spec = node->spec;
				sensor_value->data = node->data;
				sensor_value++;
				free_sensor_data_node(node);
			}

			// Send sensor message to Application thread
			k_queue_alloc_append(&shared_queue_incoming, msg);
		}

		k_sleep(K_MSEC(sensor_config->polling_period_ms));
	}
}
