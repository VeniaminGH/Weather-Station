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

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>


#include "wst_events.h"


#define LORAWAN_DEV_EUI \
	{0xDD, 0xEE, 0xAA, 0xDD, 0xBB, 0xEE, 0xEE, 0xFF}

#define LORAWAN_JOIN_EUI \
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

#define LORAWAN_APP_KEY \
	{ 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, \
	  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }

// dev_eui:  ddeeaaddbbeeeeff
// join_eui: 0000000000000000
// app_key:  2B7E151628AED2A6ABF7158809CF4F3C

#define DELAY_RETRY		K_MSEC(3000)
#define DELAY_JOIN		K_MSEC(5000)
#define NUM_OF_RETRIES	3

LOG_MODULE_REGISTER(wst_lorawan);

static void dl_callback(uint8_t port, bool data_pending,
			int16_t rssi, int8_t snr,
			uint8_t len, const uint8_t *hex_data)
{
	LOG_INF("Port %d, Pending %d, RSSI %ddB, SNR %ddBm", port, data_pending, rssi, snr);
	if (hex_data) {
		LOG_HEXDUMP_INF(hex_data, len, "Payload: ");
	}
}

static void lorwan_datarate_changed(enum lorawan_datarate dr)
{
	uint8_t next_size, max_size;

	lorawan_get_payload_sizes(&next_size, &max_size);
	LOG_DBG("New Datarate: DR_%d, Next Paylaod %d, Max Payload %d",
		dr, next_size, max_size);

	wst_event_msg_t* msg;
	msg = sys_heap_alloc(&events_pool, sizeof(wst_event_msg_t));
	if (msg == NULL) {
		LOG_ERR("couldn't alloc memory from shared pool");
		k_panic();
	}

	// Send lorawan datarate message to Application Thread
	msg->event = wst_event_lorawan_datarate;
	msg->lorawan.datarate.dr = dr;
	msg->lorawan.datarate.next_size = next_size;
	msg->lorawan.datarate.max_size = max_size;

	k_queue_alloc_append(&app_events_queue, msg);
}

static uint8_t dev_eui[] = LORAWAN_DEV_EUI;
static uint8_t join_eui[] = LORAWAN_JOIN_EUI;
static uint8_t app_key[] = LORAWAN_APP_KEY;

static struct {
	const struct device *lora_dev;
	struct lorawan_join_config join_cfg;
	struct lorawan_downlink_cb downlink_cb;
} locals = {
	.join_cfg = {
		.mode = LORAWAN_ACT_OTAA,
		.dev_eui = dev_eui,
		.otaa = {
			.join_eui = join_eui,
			.app_key = app_key,
			.nwk_key = app_key
		}
	},
	.downlink_cb = {
		.port = LW_RECV_PORT_ANY,
		.cb = dl_callback
	}
};


int wst_lorawan_join(void)
{
	int ret;

	locals.lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
	if (!device_is_ready(locals.lora_dev)) {
		LOG_ERR("%s: device not ready.", locals.lora_dev->name);
		k_panic();
	}

#if defined(CONFIG_LORAMAC_REGION_EU868)
	// Set region if more than one region is selected before calling lorawan_start()
	ret = lorawan_set_region(LORAWAN_REGION_EU868);
	if (ret < 0) {
		LOG_ERR("lorawan_set_region failed: %d", ret);
		return ret;
	}
#endif

	// Start LoRaWAN stack
	ret = lorawan_start();
	if (ret < 0) {
		LOG_ERR("lorawan_start failed: %d", ret);
		return ret;
	}

	// Register callbacks
	lorawan_register_downlink_callback(&locals.downlink_cb);
	lorawan_register_dr_changed_callback(lorwan_datarate_changed);

	// Start with 0 nonce
	locals.join_cfg.otaa.dev_nonce = 0u;

	// And try to join the Network
	int i = 1;
	do {
		LOG_INF(
			"Joining network using OTAA, dev nonce %d, attempt %d",
			locals.join_cfg.otaa.dev_nonce,
			i++);
		ret = lorawan_join(&locals.join_cfg);
		if (ret < 0) {
			if ((ret =-ETIMEDOUT))
			{
				LOG_WRN("Timed-out waiting for response.");
			} else {
				LOG_ERR("Join failed (%d)", ret);
			}
		} else {
			LOG_INF("Join successful.");
		}

		// Increment DevNonce as per LoRaWAN 1.0.4 Spec.
		locals.join_cfg.otaa.dev_nonce++;

		if (ret < 0) {
			// If failed, wait before re-trying.
			k_sleep(DELAY_JOIN);
		}

	} while (ret != 0);

#ifdef CONFIG_LORAWAN_APP_CLOCK_SYNC
	lorawan_clock_sync_run();
#endif

	return ret;
}

int wst_lorawan_send(void* data, size_t size)
{
	int ret;

	LOG_INF("Sending data: Length - %u", size);
	LOG_HEXDUMP_DBG((const uint8_t*) data, size, "Payload");

	uint32_t trials = NUM_OF_RETRIES;

	while (trials) {
		ret = lorawan_send(2, data, sizeof(data), LORAWAN_MSG_CONFIRMED);

		//
		// Note: The stack may return -EAGAIN if the provided data
		// length exceeds the maximum possible one for the region and
		// datarate. But since we are just sending the same data here,
		// we'll just continue.
		//
		if (ret == -EAGAIN) {
			LOG_ERR("LoRaWAN data send failed: %d. Continuing...", ret);
			k_sleep(DELAY_RETRY);
			trials--;
			continue;
		}

		if (ret < 0) {
			LOG_ERR("LoRaWAN data send failed: %d", ret);
		}
		else {
			LOG_INF("LoRaWAN data successfully sent!");
		}
		break;
	}
	return ret;
}
