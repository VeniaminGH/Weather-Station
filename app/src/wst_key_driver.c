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

#include "wst_key_driver.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/__assert.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wst_key_driver);

#define KEY_DEBOUNCE_MS		30

/*
 * Get key configurations from the devicetree sw0..sw2 aliases. This is mandatory.
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

typedef enum {
	WST_KEY_STATE_RELEASED,
	WST_KEY_STATE_PRESSED_DEBOUNCE,
	WST_KEY_STATE_PRESSED,
	WST_KEY_STATE_RELEASED_DEBOUNCE
} wst_key_state_t;

typedef struct {
	wst_key_id_t id;
	struct gpio_callback cb_data;
	wst_key_state_t state;
	const struct gpio_dt_spec* key;
	key_event_handler_t hanlder;
} wst_key_context_t;


static const struct gpio_dt_spec key0 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static const struct gpio_dt_spec key1 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios, {0});
static const struct gpio_dt_spec key2 = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios, {0});

static wst_key_context_t key0_ctx = {.id = WST_KEY_ID_0, .key = &key0};
static wst_key_context_t key1_ctx = {.id = WST_KEY_ID_1, .key = &key1};
static wst_key_context_t key2_ctx = {.id = WST_KEY_ID_2, .key = &key2};

static wst_key_event_t update_key_state(wst_key_context_t* key_ctx)
{
	wst_key_event_t event = WST_KEY_EVENT_NONE;

	int state = gpio_pin_get_dt(key_ctx->key);
	LOG_DBG("key%d gpio state %d", key_ctx->id, state);

	wst_key_state_t old_state = key_ctx->state;

	switch(key_ctx->state)
	{
		case WST_KEY_STATE_RELEASED:
			if (state)
			{
				key_ctx->state = WST_KEY_STATE_PRESSED_DEBOUNCE;
			}
			break;

		case WST_KEY_STATE_PRESSED_DEBOUNCE:
			if (state)
			{
				key_ctx->state = WST_KEY_STATE_PRESSED;
				event = WST_KEY_EVENT_KEY_DOWN;
			}
			else
			{
				key_ctx->state = WST_KEY_STATE_RELEASED;
			}
			break;

		case WST_KEY_STATE_PRESSED:
			if (!state)
			{
				key_ctx->state = WST_KEY_STATE_RELEASED_DEBOUNCE;
			}
			break;

		case WST_KEY_STATE_RELEASED_DEBOUNCE:
			if (state)
			{
				key_ctx->state = WST_KEY_STATE_PRESSED;
			}
			else
			{
				key_ctx->state = WST_KEY_STATE_RELEASED;
				event = WST_KEY_EVENT_KEY_UP;
			}
			break;
		
		default:
			__ASSERT(false, "Invalid key state!");
			break;
	}

	if (old_state != key_ctx->state)
	{
		LOG_DBG("key%d state: %d -> %d",
			key_ctx->id,
			old_state,
			key_ctx->state);
	}

	switch (event)
	{
		case WST_KEY_EVENT_KEY_UP:
			LOG_DBG("key%d event: KEY_UP", key_ctx->id);
			break;

		case WST_KEY_EVENT_KEY_DOWN:
			LOG_DBG("key%d event: KEY_DOWN", key_ctx->id);
			break;

		default:
			break;
	}

	return event;
}

static void key0_debounce_expired(struct k_work *work)
{
	ARG_UNUSED(work);

	wst_key_event_t event = update_key_state(&key0_ctx);

	if (key0_ctx.hanlder)
	{
		key0_ctx.hanlder(key0_ctx.id, event);
	}
}

static void key1_debounce_expired(struct k_work *work)
{
	ARG_UNUSED(work);

	wst_key_event_t event = update_key_state(&key1_ctx);

	if (key1_ctx.hanlder)
	{
		key1_ctx.hanlder(key1_ctx.id, event);
	}
}

static void key2_debounce_expired(struct k_work *work)
{
	ARG_UNUSED(work);

	wst_key_event_t event = update_key_state(&key2_ctx);

	if (key2_ctx.hanlder)
	{
		key2_ctx.hanlder(key2_ctx.id, event);
	}
}

static K_WORK_DELAYABLE_DEFINE(key0_debounce_work, key0_debounce_expired);
static K_WORK_DELAYABLE_DEFINE(key1_debounce_work, key1_debounce_expired);
static K_WORK_DELAYABLE_DEFINE(key2_debounce_work, key2_debounce_expired);

void key0_pressed(
	const struct device *dev,
	struct gpio_callback *cb,
	uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	LOG_DBG("key0 pressed at %" PRIu32 ", pins %u", k_cycle_get_32(), pins);
	update_key_state(&key0_ctx);
	k_work_reschedule(&key0_debounce_work, K_MSEC(KEY_DEBOUNCE_MS));
}

void key1_pressed(
	const struct device *dev,
	struct gpio_callback *cb,
	uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	LOG_DBG("key1 pressed at %" PRIu32 ", pins %u", k_cycle_get_32(), pins);
	update_key_state(&key1_ctx);
	k_work_reschedule(&key1_debounce_work, K_MSEC(KEY_DEBOUNCE_MS));
}

void key2_pressed(
	const struct device *dev,
	struct gpio_callback *cb,
	uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	LOG_DBG("key2 pressed at %" PRIu32 ", pins %u", k_cycle_get_32(), pins);
	update_key_state(&key2_ctx);
	k_work_reschedule(&key2_debounce_work, K_MSEC(KEY_DEBOUNCE_MS));
}

bool wst_key_driver_init(key_event_handler_t hanlder)
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

	ret = gpio_pin_interrupt_configure_dt(&key0, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			ret, key0.port->name, key0.pin);
		return false;
	}

	ret = gpio_pin_interrupt_configure_dt(&key1, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			ret, key1.port->name, key1.pin);
		return false;
	}

	ret = gpio_pin_interrupt_configure_dt(&key2, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			ret, key2.port->name, key2.pin);
		return false;
	}

	if (hanlder)
	{
		key0_ctx.hanlder = hanlder;
		key1_ctx.hanlder = hanlder;
		key2_ctx.hanlder = hanlder;
	}
	gpio_init_callback(&key0_ctx.cb_data, key0_pressed, BIT(key0.pin));
	gpio_add_callback(key0.port, &key0_ctx.cb_data);
	LOG_DBG("Set up key0 at %s pin %d", key0.port->name, key0.pin);

	gpio_init_callback(&key1_ctx.cb_data, key1_pressed, BIT(key1.pin));
	gpio_add_callback(key1.port, &key1_ctx.cb_data);
	LOG_DBG("Set up key1 at %s pin %d", key1.port->name, key1.pin);

	gpio_init_callback(&key2_ctx.cb_data, key2_pressed, BIT(key2.pin));
	gpio_add_callback(key2.port, &key2_ctx.cb_data);
	LOG_DBG("Set up key2 at %s pin %d", key2.port->name, key2.pin);

	LOG_INF("Driver is successfully initialized");

	return true;
}
