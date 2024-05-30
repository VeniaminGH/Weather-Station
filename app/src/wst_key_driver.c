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

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wst_key_driver);

/*
 * Get key0 configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#define SW1_NODE	DT_ALIAS(sw1)
#define SW2_NODE	DT_ALIAS(sw2)

#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
#if !DT_NODE_HAS_STATUS(SW1_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#if !DT_NODE_HAS_STATUS(SW2_NODE, okay)
#error "Unsupported board: sw2 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec key0 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static const struct gpio_dt_spec key1 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios, {0});
static const struct gpio_dt_spec key2 = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios, {0});

static struct gpio_callback key0_cb_data;
static struct gpio_callback key1_cb_data;
static struct gpio_callback key2_cb_data;

void key0_pressed(
	const struct device *dev,
	struct gpio_callback *cb,
	uint32_t pins)
{
	LOG_DBG("key0 pressed at %" PRIu32 "", k_cycle_get_32());
	LOG_INF("key0 pressed");
}

void key1_pressed(
	const struct device *dev,
	struct gpio_callback *cb,
	uint32_t pins)
{
	LOG_DBG("key1 pressed at %" PRIu32 "", k_cycle_get_32());
	LOG_INF("key1 pressed");
}

void key2_pressed(
	const struct device *dev,
	struct gpio_callback *cb,
	uint32_t pins)
{
	LOG_DBG("key2 pressed at %" PRIu32 "", k_cycle_get_32());
	LOG_INF("key2 pressed");
}

bool wst_key_driver_init(void)
{
	LOG_INF("Driver is initializing ...");

	if (!gpio_is_ready_dt(&key0)) {
		LOG_ERR("key0 device %s is not ready", key0.port->name);
		return false;
	}
	if (!gpio_is_ready_dt(&key1)) {
		LOG_ERR("key1 device %s is not ready", key1.port->name);
		return false;
	}
	if (!gpio_is_ready_dt(&key2)) {
		LOG_ERR("key2 device %s is not ready", key2.port->name);
		return false;
	}

	int ret = gpio_pin_configure_dt(&key0, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d",
			ret, key0.port->name, key0.pin);
		return false;
	}

	ret = gpio_pin_configure_dt(&key1, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d",
			ret, key1.port->name, key1.pin);
		return false;
	}

	ret = gpio_pin_configure_dt(&key2, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d",
			ret, key2.port->name, key2.pin);
		return false;
	}

	ret = gpio_pin_interrupt_configure_dt(&key0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			ret, key0.port->name, key0.pin);
		return false;
	}

	ret = gpio_pin_interrupt_configure_dt(&key1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			ret, key1.port->name, key1.pin);
		return false;
	}

	ret = gpio_pin_interrupt_configure_dt(&key2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			ret, key2.port->name, key2.pin);
		return false;
	}

	gpio_init_callback(&key0_cb_data, key0_pressed, BIT(key0.pin));
	gpio_add_callback(key0.port, &key0_cb_data);
	LOG_DBG("Set up key0 at %s pin %d", key0.port->name, key0.pin);

	gpio_init_callback(&key1_cb_data, key1_pressed, BIT(key1.pin));
	gpio_add_callback(key1.port, &key1_cb_data);
	LOG_DBG("Set up key1 at %s pin %d", key1.port->name, key1.pin);

	gpio_init_callback(&key2_cb_data, key2_pressed, BIT(key2.pin));
	gpio_add_callback(key2.port, &key2_cb_data);
	LOG_DBG("Set up key2 at %s pin %d", key2.port->name, key2.pin);

	LOG_INF("Driver is successfully initialized");

	return true;
}
