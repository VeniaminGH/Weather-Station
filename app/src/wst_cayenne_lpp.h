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
 *
 * This module is based on below specification:
 *
 *   https://docs.mydevices.com/docs/lorawan/cayenne-lpp
 *
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#define IPSO_OBJECT_ID_BASE					(3200)

//
// Values are taken from https://omaspecworks.org/wp-content/uploads/2018/03/ipso-paper.pdf
//

// IPSO Starter Pack Objects

#define IPSO_OBJECT_ID_DIGITAL_INPUT		(3200)	// Digital Input
#define IPSO_OBJECT_ID_DIGITAL_OUTPUT		(3201)	// Digital Output
#define IPSO_OBJECT_ID_ANALOG_INPUT			(3202)	// Analogue Input
#define IPSO_OBJECT_ID_ANALOG_OUTPUT		(3203)	// Analogue Output
#define IPSO_OBJECT_ID_GENERIC_SENSOR		(3300)	// Generic Sensor
#define IPSO_OBJECT_ID_ILLUMINANCE_SENSOR	(3301)	// Illuminance Sensor
#define IPSO_OBJECT_ID_PRESENCE_SENSOR		(3302)	// Presence Sensor
#define IPSO_OBJECT_ID_TEMPERATURE_SENSOR	(3303)	// Temperature Sensor
#define IPSO_OBJECT_ID_HUMIDITY_SENOSR		(3304)	// Humidity Sensor
#define IPSO_OBJECT_ID_POWER_MEASUREMENT	(3305)	// Power Measurement
#define IPSO_OBJECT_ID_ACTUATION			(3306)	// Actuation
#define IPSO_OBJECT_ID_SET_POINT			(3308)	// Set Point
#define IPSO_OBJECT_ID_LOAD_CONTROL			(3310)	// Load Control
#define IPSO_OBJECT_ID_LIGHT_CONTROL		(3311)	// Light Control
#define IPSO_OBJECT_ID_POWER_CONTROL		(3312)	// Power Control
#define IPSO_OBJECT_ID_ACCELEROMETER		(3313)	// Accelerometer
#define IPSO_OBJECT_ID_MAGNOMETER			(3314)	// Magnetometer
#define IPSO_OBJECT_ID_BAROMETER			(3315)	// Barometer

// IPSO Expansion Pack Objects

#define IPSO_OBJECT_ID_VOLTAGE				(3316)	// Voltage
#define IPSO_OBJECT_ID_CURRENT				(3317)	// Current
#define IPSO_OBJECT_ID_FREQUENCY			(3318)	// Frequency
#define IPSO_OBJECT_ID_DEPTH				(3319)	// Depth
#define IPSO_OBJECT_ID_PERCENTAGE			(3320)	// Percentage
#define IPSO_OBJECT_ID_ALTITUDE				(3321)	// Altitude
#define IPSO_OBJECT_ID_LOAD					(3322)	// Load
#define IPSO_OBJECT_ID_PRESSURE				(3323)	// Pressure
#define IPSO_OBJECT_ID_LOUDNESS				(3324)	// Loudness
#define IPSO_OBJECT_ID_CONCENTRATION		(3325)	// Concentration
#define IPSO_OBJECT_ID_ACIDITY				(3326)	// Acidity
#define IPSO_OBJECT_ID_CONDUCTIVITY			(3327)	// Conductivity
#define IPSO_OBJECT_ID_POWER				(3328)	// Power
#define IPSO_OBJECT_ID_POWER_FACTOR			(3329)	// Power Factor
#define IPSO_OBJECT_ID_DISTANCE				(3330)	// Distance
#define IPSO_OBJECT_ID_ENERGY				(3331)	// Energy
#define IPSO_OBJECT_ID_DIRECTION			(3332)	// Direction
#define IPSO_OBJECT_ID_TIME					(3333)	// Time
#define IPSO_OBJECT_ID_GYROMETER			(3334)	// Gyrometer
#define IPSO_OBJECT_ID_COLOR				(3335)	// Color
#define IPSO_OBJECT_ID_GPS_LOCATION			(3336)	// GPS Location
#define IPSO_OBJECT_ID_POSITIONER			(3337)	// Positioner
#define IPSO_OBJECT_ID_BUZZER				(3338)	// Buzzer
#define IPSO_OBJECT_ID_AUDIO_CLIP			(3339)	// Audio Clip
#define IPSO_OBJECT_ID_TIMER				(3340)	// Timer
#define IPSO_OBJECT_ID_ADDR_TEXT_DISPLAY	(3341)	// Addressable Text Display
#define IPSO_OBJECT_ID_ONOFF_SWITCH			(3342)	// On/Off Switch
#define IPSO_OBJECT_ID_LEVEL_CONTROL		(3343)	// Level Control
#define IPSO_OBJECT_ID_UPDOWN_CONTROL		(3344)	// Up/Down Control
#define IPSO_OBJECT_ID_MULTI_AXIS_JOYSTICK	(3345)	// Multiple Axis Joystick
#define IPSO_OBJECT_ID_RATE					(3346)	// Rate
#define IPSO_OBJECT_ID_PUSH_BTTON			(3347)	// Push Button
#define IPSO_OBJECT_ID_MULTISTATE_SELECTOR	(3348)	// Multistate Selector


#define CAYENNE_LPP_TYPE(ipso_id)				((ipso_id) - IPSO_OBJECT_ID_BASE)


/**
 * @brief Opaque Cayenne LPP context
 */
typedef struct cayenne_lpp_stream cayenne_lpp_stream_t;

/**
 * @brief Cayenne LPP result type
 */
typedef enum {
	cayenne_lpp_result_success = 0,					//< success
	cayenne_lpp_result_error_unknown_type = 1,		//< unknown data type
	cayenne_lpp_result_error_not_implemented = 2,	//< unsuported data type
	cayenne_lpp_result_error_overflow = 3,			//< no free space in encoding stream
	cayenne_lpp_result_error_end_of_stream = 4		//< end of encoding stream
} cayenne_lpp_result_t;

/**
 * @brief Cayenne LPP data type
 */
typedef enum {
	cayenne_lpp_type_digital_input			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_DIGITAL_INPUT),
	cayenne_lpp_type_digital_output			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_DIGITAL_OUTPUT),
	cayenne_lpp_type_analog_input			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ANALOG_INPUT),
	cayenne_lpp_type_analog_output			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ANALOG_OUTPUT),
	cayenne_lpp_type_generic_sensor			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_GENERIC_SENSOR),
	cayenne_lpp_type_illuminance_sensor		= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ILLUMINANCE_SENSOR),
	cayenne_lpp_type_presence_sensor		= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_PRESENCE_SENSOR),
	cayenne_lpp_type_temperature_sensor		= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_TEMPERATURE_SENSOR),
	cayenne_lpp_type_humiditiy_sensor		= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_HUMIDITY_SENOSR),
	cayenne_lpp_type_power_measurement		= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_POWER_MEASUREMENT),
	cayenne_lpp_type_actuation				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ACTUATION),
	cayenne_lpp_type_set_point				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_SET_POINT),
	cayenne_lpp_type_load_control			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_LOAD_CONTROL),
	cayenne_lpp_type_light_control			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_LIGHT_CONTROL),
	cayenne_lpp_type_power_control			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_POWER_CONTROL),
	cayenne_lpp_type_accelerometer			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ACCELEROMETER),
	cayenne_lpp_type_magnometer				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_MAGNOMETER),
	cayenne_lpp_type_barometer				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_BAROMETER),
	cayenne_lpp_type_voltage				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_VOLTAGE),
	cayenne_lpp_type_current				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_CURRENT),
	cayenne_lpp_type_frequency				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_FREQUENCY),
	cayenne_lpp_type_depth					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_DEPTH),
	cayenne_lpp_type_percentage				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_PERCENTAGE),
	cayenne_lpp_type_altitude				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ALTITUDE),
	cayenne_lpp_type_load					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_LOAD),
	cayenne_lpp_type_pressure				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_PRESSURE),
	cayenne_lpp_type_loudness				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_LOUDNESS),
	cayenne_lpp_type_concentration			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_CONCENTRATION),
	cayenne_lpp_type_acidity				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ACIDITY),
	cayenne_lpp_type_conductivity			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_CONDUCTIVITY),
	cayenne_lpp_type_power					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_POWER),
	cayenne_lpp_type_power_factor			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_POWER_FACTOR),
	cayenne_lpp_type_distance				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_DISTANCE),
	cayenne_lpp_type_energy					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ENERGY),
	cayenne_lpp_type_direction				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_DIRECTION),
	cayenne_lpp_type_time					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_TIME),
	cayenne_lpp_type_gyrometer				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_GYROMETER),
	cayenne_lpp_type_color					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_COLOR),
	cayenne_lpp_type_gps_location			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_GPS_LOCATION),
	cayenne_lpp_type_positioner				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_POSITIONER),
	cayenne_lpp_type_buzzer					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_BUZZER),
	cayenne_lpp_type_audio_clip				= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_AUDIO_CLIP),
	cayenne_lpp_type_timer					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_TIMER),
	cayenne_lpp_type_addr_text_display		= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ADDR_TEXT_DISPLAY),
	cayenne_lpp_type_onoff_switch			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_ONOFF_SWITCH),
	cayenne_lpp_type_level_control			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_LEVEL_CONTROL),
	cayenne_lpp_type_updown_control			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_UPDOWN_CONTROL),
	cayenne_lpp_type_multi_axis_control		= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_MULTI_AXIS_JOYSTICK),
	cayenne_lpp_type_rate					= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_RATE),
	cayenne_lpp_type_push_button			= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_PUSH_BTTON),
	cayenne_lpp_type_multistate_selector	= CAYENNE_LPP_TYPE(IPSO_OBJECT_ID_MULTISTATE_SELECTOR)
} cayenne_lpp_type_t;

/**
 * @brief Cayenne LPP value type
 */
typedef union {
		uint8_t digital_input;		//< 1 byte
		uint8_t digital_output;		//< 1 byte

		float analog_input;			//< 2 bytes, signed, 0.01
		float analog_output;		//< 2 bytes, signed, 0.01

		struct {
			float celsius;			//< 2 bytes, signed, 0.1
		} temperature_sensor;

		struct {
			float rh;
		} humidity_sensor;

		struct {
			float x;
			float y;
			float z;
		} accelerometer;

		struct {
			float hpa;
		} barometer;

		struct {
			float x;
			float y;
			float z;
		} gyrometer;

		struct {
			float x;
			float y;
			float z;
		} gps_location;
} cayenne_lpp_value_t;


/**
 * @brief Creates new stream for encoding or decoding.
 *
 * Creats cayenne stream for encoding/decoding with the given size.
 * If buffer is not provided, the stream is created for encoding.
 * If provided, the stream is created for decoding from the given buffer.
 *
 * @param[in] size        maximum stream size
 * @param[in] buffer      optional buffer pointer, may be NULL
 *
 * @return Opaque stream pointer or NULL
 */
cayenne_lpp_stream_t* cayenne_lpp_stream_new(size_t size, uint8_t* buffer);

/**
 * @brief Deletes given stream and deallocates all its resources.
 *
 * @param[in] stream      opaque pointer to the stream
 */
void cayenne_lpp_stream_delete(cayenne_lpp_stream_t* stream);


/**
 * @brief Writes value to encoding stream.
 *
 * The stream must be opened in encoding mode.
 *
 * @param[in] stream      opaque pointer to the stream
 * @param[in] channel     the data channel to write
 * @param[in] type        the data type to write
 * @param[in] value       pointer to data value to write
 *
 * @return @cayenne_lpp_result_success on success or one of the error codes:
 *
 *   cayenne_lpp_result_error_unknown_type:    unknown data type
 *   cayenne_lpp_result_error_not_implemented: unsuported data type
 *   cayenne_lpp_result_error_overflow:        no free space in the stream
 */
cayenne_lpp_result_t cayenne_lpp_stream_write(
	cayenne_lpp_stream_t* stream,
	uint8_t channel,
	cayenne_lpp_type_t type,
	const cayenne_lpp_value_t* value);


/**
 * @brief Reads value from decoding stream.
 *
 * The stream must be opened in decoding mode.
 *
 * @param[in]  stream      opaque pointer to the stream
 * @param[out] channel    read data channel
 * @param[out] type       read data type
 * @param[out] value      read data value
 *
 * @return @cayenne_lpp_result_success on success or one of the error codes:
 *
 *   cayenne_lpp_result_error_unknown_type:    unknown data type
 *   cayenne_lpp_result_error_not_implemented: unsuported data type,
 *   cayenne_lpp_result_error_end_of_stream:   end of encoding stream
 */
cayenne_lpp_result_t cayenne_lpp_stream_read(
	cayenne_lpp_stream_t* stream,
	uint8_t* channel,
	cayenne_lpp_type_t* type,
	cayenne_lpp_value_t* value);


/**
 * @brief Returns encoding/decoding stream buffer.
 *
 * @param[in]  stream      opaque pointer to the stream
 * @param[out] buffer_size stream buffer size
 * @param[out] stream_size current stream size
 * @param[out] value       read data value
 *
 * @return Buffer pointer.
 *
 * Decoding stream buffer and stream sizes are equal.
 */
const uint8_t* cayenne_lpp_stream_get_buffer(
	cayenne_lpp_stream_t* stream,
	size_t* buffer_size,
	size_t* stream_size);


/**
 * @brief Returns encoding/decoding stream free space.
 *
 * @param[in] stream      opaque pointer to the stream
 *
 * @return Stream free space.
 *
 * Decoding stream free space is zero.
 */
size_t cayenne_lpp_stream_get_free_space(
	cayenne_lpp_stream_t* stream);


/**
 * @brief Resets encoding/decoding stream state
 *
 * Rewinds encoding stream write pointer and flushes all encoded data.
 * Rewinds decoding stream read pointer.
 * 
 * @param[in] stream      opaque pointer to the stream
 */
void cayenne_lpp_stream_reset(
	cayenne_lpp_stream_t* stream);
