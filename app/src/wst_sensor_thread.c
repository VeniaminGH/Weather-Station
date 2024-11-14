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
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/dsp/print_format.h>


LOG_MODULE_REGISTER(wst_sensor_thread);

#define DELAY_SENSOR_INTERVAL	K_MSEC(5000)

#define NUM_SENSORS 3
#define NUM_SENSOR_CHANNELS 5

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
	{SENSOR_CHAN_PRESS, 0}
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

static bool wst_sensor_get_data(struct sensor_q31_data* sensor_data, uint16_t count)
{
	int rc;
	const struct sensor_decoder_api *decoder;
	struct rtio_cqe *cqe;
	uint8_t *buf;
	uint32_t buf_len;

	// Non-Blocking read for each sensor
	for (int i = 0; i < NUM_SENSORS; i++) {
		rc = sensor_read_async_mempool(iodevs[i], &temp_ctx, iodevs[i]);

		if (rc != 0) {
			LOG_ERR("sensor_read() failed %d\n", rc);
			return false;
		}
	}

	// Wait for read completions
	for (int i = 0; i < NUM_SENSORS; i++) {
		cqe = rtio_cqe_consume_block(&temp_ctx);

		if (cqe->result != 0) {
			LOG_ERR("async read failed %d\n", cqe->result);
			return false;
		}

		// Get the associated mempool buffer with the completion
		rc = rtio_cqe_get_mempool_buffer(&temp_ctx, cqe, &buf, &buf_len);

		if (rc != 0) {
			LOG_ERR("get mempool buffer failed %d\n", rc);
			return false;
		}

		const struct device *sensor = ((const struct sensor_read_config *)
			((struct rtio_iodev *)cqe->userdata)->data)->sensor;

		// Done with the completion event, release it
		rtio_cqe_release(&temp_ctx, cqe);

		rc = sensor_get_decoder(sensor, &decoder);
		if (rc != 0) {
			LOG_ERR("sensor_get_decoder failed %d\n", rc);
			return false;
		}

		switch(i) {
			case 0:	{
					// Frame iterators, one per channel we are decoding
					uint32_t fits = 0;
					rc = decoder->decode(
						buf,
						(struct sensor_chan_spec) {SENSOR_CHAN_DIE_TEMP, 0},
						&fits,
						1,
						&sensor_data[0]
					);

					LOG_DBG("Die Temperature for %s channel 0, result - %d, value - %" PRIsensor_q31_data,
						die_temp_sensor->name,
						rc,
						PRIsensor_q31_data_arg(sensor_data[0], 0)
					);
				}
				break;

			case 1: {
					// Frame iterators, one per channel we are decoding
					uint32_t fits = 0;
					rc = decoder->decode(
						buf,
						(struct sensor_chan_spec) {SENSOR_CHAN_LIGHT, 0},
						&fits,
						1,
						&sensor_data[1]
					);

					LOG_DBG("Illuminance for %s channel 0, result - %d, value - %" PRIsensor_q31_data,
						light_sensor->name,
						rc,
						PRIsensor_q31_data_arg(sensor_data[1], 0)
					);
				}
				break;

			case 2: {
					// Frame iterators, one per channel we are decoding
					uint32_t fits = 0;
					rc = decoder->decode(
						buf,
						(struct sensor_chan_spec) {SENSOR_CHAN_AMBIENT_TEMP, 0},
						&fits,
						1,
						&sensor_data[2]
					);

					LOG_DBG("Ambient Temperature for %s channel 0, result - %d, value - %" PRIsensor_q31_data,
						env_sensor->name,
						rc,
						PRIsensor_q31_data_arg(sensor_data[2], 0)
					);

					fits = 0;
					rc = decoder->decode(
						buf,
						(struct sensor_chan_spec) {SENSOR_CHAN_HUMIDITY, 0},
						&fits,
						1,
						&sensor_data[3]
					);

					LOG_DBG("Humidity for %s channel 0, result - %d, value - %" PRIsensor_q31_data,
						env_sensor->name,
						rc,
						PRIsensor_q31_data_arg(sensor_data[3], 0)
					);

					fits = 0;
					rc = decoder->decode(
						buf,
						(struct sensor_chan_spec) {SENSOR_CHAN_PRESS, 0},
						&fits,
						1,
						&sensor_data[4]
					);

					LOG_DBG("Pressure for %s channel 0, result - %d, value - %" PRIsensor_q31_data,
						env_sensor->name,
						rc,
						PRIsensor_q31_data_arg(sensor_data[4], 0)
					);
				}
				break;

			default:
				break;
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

		struct sensor_q31_data temp_data[NUM_SENSOR_CHANNELS] = {0};

		if (wst_sensor_get_data(temp_data, NUM_SENSOR_CHANNELS)) {
			wst_event_msg_t* msg;

			msg = sys_heap_alloc(&shared_pool, sizeof(wst_event_msg_t) + sizeof(temp_data));
			if (msg == NULL) {
				LOG_ERR("couldn't alloc memory from shared pool");
				k_panic();
			}

			// Send Sensor message to Application thread
			msg->event = wst_event_sensor_data_available;
			msg->sensor.count = NUM_SENSOR_CHANNELS;
			memcpy(msg->sensor.data, temp_data, sizeof(temp_data));
			k_queue_alloc_append(&shared_queue_incoming, msg);
		}

		k_sleep(DELAY_SENSOR_INTERVAL);
	}
}
