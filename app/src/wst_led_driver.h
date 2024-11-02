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

#define WST_LED_DRIVER_NAME	"WST_LED_DRIVER"

typedef enum {
	WST_LED_ID_RED,
	WST_LED_ID_GREEN,
	WST_LED_ID_BLUE
} wst_led_id_t;

typedef int (*wst_led_driver_set_state_t)(
	const struct device *dev,
	wst_led_id_t id,
	bool state);


__subsystem struct wst_led_driver_api {
	wst_led_driver_set_state_t set_state;
};

/*
 * Set LED state to ON/OFF
 *
 * @param dev		LED driver device
 * @param id		LED id
 * @param state		true - ON, false - OFF
 */
__syscall int wst_led_driver_set_state(
	const struct device *dev,
	wst_led_id_t id,
	bool state);

static inline int z_impl_wst_led_driver_set_state(
	const struct device *dev,
	wst_led_id_t id,
	bool state)
{
	const struct wst_led_driver_api *api = dev->api;

	return api->set_state(dev, id, state);
}

#include <zephyr/syscalls/wst_led_driver.h>
