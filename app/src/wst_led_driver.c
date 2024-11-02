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

#include "wst_led_driver.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/__assert.h>

DEVICE_DECLARE(wst_led_driver_0);

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wst_led_driver);

/*
 * The led devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
static struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
static struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});

struct wst_led_driver_dev_data {
	const struct device *dev;
};

static struct wst_led_driver_dev_data wst_led_driver_dev_data_0;


static int wst_led_driver_init_impl(const struct device *dev)
{
	int ret = 0;

	struct wst_led_driver_dev_data *data = dev->data;

	LOG_INF("LED Driver is initializing ...");

	if (led_blue.port && !gpio_is_ready_dt(&led_blue)) {
		LOG_ERR("LED device %s is not ready; ignoring it\n", led_blue.port->name);
		led_blue.port = NULL;
	}
	if (led_green.port && !gpio_is_ready_dt(&led_green)) {
		LOG_ERR("LED device %s is not ready; ignoring it\n", led_green.port->name);
		led_green.port = NULL;
	}
	if (led_red.port && !gpio_is_ready_dt(&led_red)) {
		LOG_ERR("LED device %s is not ready; ignoring it\n", led_red.port->name);
		led_red.port = NULL;
	}

	if (!ret && led_blue.port) {
		ret = gpio_pin_configure_dt(&led_blue, GPIO_OUTPUT);
		if (ret != 0) {
			LOG_ERR("Error %d: failed to configure LED device %s pin %d\n",
				ret,
				led_blue.port->name,
				led_blue.pin
			);
			led_blue.port = NULL;
		} else {
			LOG_DBG("Set up LED at %s pin %d\n", led_blue.port->name, led_blue.pin);
		}
	}
	if (!ret && led_green.port) {
		ret = gpio_pin_configure_dt(&led_green, GPIO_OUTPUT);
		if (ret != 0) {
			LOG_ERR("Error %d: failed to configure LED device %s pin %d\n",
				ret,
				led_green.port->name,
				led_green.pin);
			led_green.port = NULL;
		} else {
			LOG_DBG("Set up LED at %s pin %d\n", led_green.port->name, led_green.pin);
		}
	}
	if (!ret && led_red.port) {
		ret = gpio_pin_configure_dt(&led_red, GPIO_OUTPUT);
		if (ret != 0) {
			LOG_ERR("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led_red.port->name, led_red.pin);
			led_red.port = NULL;
		} else {
			LOG_DBG("Set up LED at %s pin %d\n", led_red.port->name, led_red.pin);
		}
	}

	if (!ret && led_blue.port && led_green.port && led_red.port)
	{
		data->dev = dev;
		LOG_INF("LED Driver is successfully initialized - %p", dev);
	}
	else
	{
		LOG_ERR("LED Driver initialization failed!");
	}
	return ret;
}

static int wst_led_driver_set_state_impl(
	const struct device *dev,
	wst_led_id_t id,
	bool state)
{
	int ret = -EINVAL;

	switch (id)
	{
		case WST_LED_ID_BLUE:
			if (led_blue.port)
			{
				ret = gpio_pin_set_dt(&led_blue, state);
			}
			break;

		case WST_LED_ID_GREEN:
			if (led_green.port)
			{
				ret = gpio_pin_set_dt(&led_green, state);
			}
			break;

		case WST_LED_ID_RED:
			if (led_red.port)
			{
				ret = gpio_pin_set_dt(&led_red, state);
			}
			break;

		default:
			__ASSERT(false, "Invalid led id!");
			break;
	}

	if (ret)
	{
		LOG_ERR("wst_led_driver_set_led failed, error code - %d!", ret);
	}

	return ret;
}

static struct wst_led_driver_api wst_led_driver_api = {
	.set_state = wst_led_driver_set_state_impl
};

DEVICE_DEFINE(wst_led_driver_0, WST_LED_DRIVER_NAME,
	&wst_led_driver_init_impl, NULL,
	&wst_led_driver_dev_data_0, NULL,
	POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
	&wst_led_driver_api);
