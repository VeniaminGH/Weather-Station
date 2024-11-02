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

#include <stdint.h>

#include <zephyr/device.h>

#define WST_KEY_DRIVER_NAME	"WST_KEY_DRIVER"

typedef enum {
	WST_KEY_ID_0,
	WST_KEY_ID_1,
	WST_KEY_ID_2,
} wst_key_id_t;

typedef enum {
	WST_KEY_EVENT_NONE,
	WST_KEY_EVENT_KEY_UP,
	WST_KEY_EVENT_KEY_DOWN,
	WST_KEY_EVENT_KEY_SINGLE_PRESS
} wst_key_event_t;

typedef void (*key_event_handler_t)(
	const struct device *dev,
	wst_key_id_t id,
	wst_key_event_t event,
	void* context);

typedef int (*wst_key_driver_set_handler_t)(
	const struct device *dev,
	key_event_handler_t handler,
	void *context);

__subsystem struct wst_key_driver_api {
	wst_key_driver_set_handler_t set_handler;
};


/*
 * Register key driver handler callback
 *
 * This callback runs in a system worker queue context.
 *
 * @param dev		Key driver device to install handler callback
 * @param handler	Hanlder callback pointer
 * @param context	Context passed to handler callback, or NULL if not needed

 * @return 0 Success, nonzero if an error occurred
 */
__syscall int wst_key_driver_set_handler(
	const struct device *dev,
	key_event_handler_t handler,
	void *context);

static inline int z_impl_wst_key_driver_set_handler(
	const struct device *dev,
	key_event_handler_t handler,
	void *context)
{
	const struct wst_key_driver_api *api = dev->api;

	return api->set_handler(dev, handler, context);
}

#include <zephyr/syscalls/wst_key_driver.h>
