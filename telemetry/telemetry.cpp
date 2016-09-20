/*
 * This file is part of MultiOSD <https://github.com/UncleRus/MultiOSD>
 *
 * MultiOSD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "telemetry.h"
#include "../lib/uart/uart.h"
#include <avr/eeprom.h>
#include <math.h>
#include <string.h>
#include "../lib/timer/timer.h"

namespace telemetry
{

void init ()
{
	::settings::append_section (__settings, sizeof (__settings) / sizeof (::settings::option_t));

	for (uint8_t i = 0; i < modules::count; i ++)
		modules::init_settings (i);
}

const char default_callsign [] PROGMEM = TELEMETRY_DEFAULT_CALLSIGN;

void reset ()
{
	eeprom_update_float (EEPROM_ADDR_MIN_CELL_VOLTAGE, TELEMETRY_DEFAULT_BATTERY_MIN_CELL_VOLTAGE);
	eeprom_update_float (EEPROM_ADDR_NOM_CELL_VOLTAGE, TELEMETRY_DEFAULT_BATTERY_NOM_CELL_VOLTAGE);
	eeprom_update_float (EEPROM_ADDR_MAX_CELL_VOLTAGE, TELEMETRY_DEFAULT_BATTERY_MAX_CELL_VOLTAGE);
	eeprom_update_float (EEPROM_ADDR_LOW_VOLTAGE, TELEMETRY_DEFAULT_BATTERY_LOW_CELL_VOLTAGE);
	eeprom_update_byte (EEPROM_ADDR_RSSI_LOW_THRESHOLD, TELEMETRY_DEFAULT_RSSI_LOW_THRESHOLD);
	for (uint8_t i = 0; i < TELEMETRY_CALLSIGN_LENGTH; i ++)
		eeprom_update_byte ((uint8_t *) EEPROM_ADDR_CALLSIGN + i, pgm_read_byte (&default_callsign [i]));
	eeprom_update_byte ((uint8_t *) EEPROM_ADDR_CALLSIGN + TELEMETRY_CALLSIGN_LENGTH, 0);

	eeprom_update_block (TELEMETRY_DEFAULT_CALLSIGN, EEPROM_ADDR_CALLSIGN, TELEMETRY_CALLSIGN_LENGTH);

	for (uint8_t i = 0; i < modules::count; i ++)
		modules::reset_settings (i);
}

}  // namespace settings

///////////////////////////////////////////////////////////////////////////////

uint32_t update_time = 0;
settings::settings_t s;

namespace status
{

	const char *callsign;
	connection_state_t connection = DISCONNECTED;
	uint16_t flight_time = 0;
	uint8_t flight_mode = 0;
	const char *flight_mode_name_p = NULL;
	const char *flight_mode_name = NULL;
	bool armed = false;

}  // namespace status

namespace attitude
{

	float roll = 0;
	float pitch = 0;
	float yaw = 0;

}  // namespace attitude

namespace input
{

	uint8_t rssi = 0;
	bool connected = false;
	bool rssi_low;
	int8_t throttle = 0;
	int8_t roll = 0;
	int8_t pitch = 0;
	int8_t yaw = 0;
	uint16_t channels [INPUT_CHANNELS] = {0};

	void set_rssi (uint8_t value)
	{
		rssi = value;
		rssi_low = value < s.rssi_low_thresh;
	}

}  // namespace input

namespace gps
{

	float latitude = 0.0;
	float longitude = 0.0;
	float altitude = 0.0;
	float speed = 0.0;
	uint16_t heading = 0;
	int8_t satellites = 0;
	gps_state_t state = GPS_STATE_NO_FIX;
	float climb = 0.0;

	float hdop = 99.99;
	float vdop = 99.99;
	float pdop = 99.99;
}  // namespace gps

namespace barometer
{

	float altitude = 0.0;
	float temperature = 0.0;
	uint32_t pressure = 0;

}  // namespace barometer

namespace environment
{

	uint16_t wind_direction = 0;
	float wind_speed = 0.0;
	float temperature = 0.0;

}  // namespace wind

namespace stable
{

	float climb = 0.0;
	float altitude = 0.0;
	float groundspeed = 0.0;
	float airspeed = 0.0;
	uint16_t heading = 0;
	heading_source_t heading_source = DISABLED;

	uint32_t _alt_update_time = 0;

	void update_alt_climb (float _alt)
	{
		climb = (_alt - altitude) / (update_time - _alt_update_time) * 1000;
		altitude = _alt;
		_alt_update_time = update_time;
	}

	void calc_heading (float x, float y)
	{
		int16_t bearing = 90 + atan2 (x, y) * 57.295775;

		// TODO: declination

		if (bearing < 0) bearing += 360;    // normalization
		if (bearing > 360) bearing -= 360;  // normalization
		heading = bearing;
	}

}  // namespace stable

namespace battery
{

	float _cell_range;

	battery_t battery1;
	battery_t battery2;

	void battery_t::set_voltage (float value, bool calc_cells)
	{
		voltage = value;

		if (calc_cells) cells = round (voltage / s.nom_cell_voltage);
		if (!cells)
		{
			cell_voltage = 0;
			level = 0;
			low = true;
			return;
		}
		cell_voltage = voltage / cells;
		low = cell_voltage <= s.low_cell_voltage;
		level = cell_voltage > s.min_cell_voltage
			? (cell_voltage - s.min_cell_voltage) / _cell_range * 100
			: 0;
		if (level > 100) level = 100;
	}

	void battery_t::set_amperage (float value, uint16_t interval)
	{
		amperage = value;
		consumed += amperage * interval / 3600.0;
	}

	void init ()
	{
		_cell_range = s.max_cell_voltage - s.min_cell_voltage;
	}

}  // namespace battery

namespace home
{

	home_state_t state = NO_FIX;

	float distance = 0;
	uint16_t direction = 0;

	float longitude;
	float latitude;
	float altitude = 0;

	void fix ()
	{
		state = FIXING;
	}

	void update ()
	{
		if (state == NO_FIX) return;
		if (state == FIXING)
			switch (gps::state)
			{
				case GPS_STATE_NO_FIX:
					state = NO_FIX;
					break;
				case GPS_STATE_FIXING:
				case GPS_STATE_2D:
					if (state == NO_FIX) state = FIXING;
					break;
				case GPS_STATE_3D:
					state = FIXED;
					longitude = gps::longitude;
					latitude = gps::latitude;
					altitude = stable::altitude;
					break;
			}
		if (state != FIXED) return;

		float rads = fabs (latitude) * 0.0174532925;
		double scale_down = cos (rads);
		double scale_up = 1.0 / cos (rads);

		// distance
		float dstlon = fabs (longitude - gps::longitude) * 111319.5 * scale_down;
		float dstlat = fabs (latitude - gps::latitude) * 111319.5;
		distance = sqrt (square (dstlon) + square (dstlat));

		// DIR to Home
		dstlon = longitude - gps::longitude; 				// x offset
		dstlat = (latitude - gps::latitude) * scale_up; 	// y offset
		int16_t bearing = 90 + atan2 (dstlat, -dstlon) * 57.295775; // absolute home direction
		if (bearing < 0) bearing += 360;	// normalization
		bearing -= 180;						// absolute return direction
		if (bearing < 0) bearing += 360;	// normalization
		bearing -= stable::heading;			// relative home direction
		if (bearing < 0) bearing += 360;	// normalization
		direction = bearing;
	}

}  // namespace home

namespace waypoint
{

	uint8_t prev = 0;
	uint8_t current = 0;
	uint16_t distance = 0;
	uint16_t bearing = 0;

	void set_bearing (int16_t value)
	{
		bearing = value < 0 ? value + 360 : value;
		if (bearing >= 360) bearing -= 360;
	}

	void set (uint8_t value)
	{
		if (current == value) return;
		prev = current;
		current = value;
	}

}  // namespace waypoint

}  // namespace telemetry

#ifdef TELEMETRY_MODULES_ADC_BATTERY
	#include "adc_battery/adc_battery.h"
#endif
#ifdef TELEMETRY_MODULES_ADC_RSSI
	#include "adc_rssi/adc_rssi.h"
#endif
#ifdef TELEMETRY_MODULES_I2C_BARO
	#include "i2c_baro/i2c_baro.h"
#endif
#ifdef TELEMETRY_MODULES_I2C_COMPASS
	#include "i2c_compass/i2c_compass.h"
#endif
#ifdef TELEMETRY_MODULES_UAVTALK
	#include "uavtalk/uavtalk.h"
#endif
#ifdef TELEMETRY_MODULES_MAVLINK
	#include "mavlink/mavlink.h"
#endif
#ifdef TELEMETRY_MODULES_UBX
	#include "ubx/ubx.h"
#endif
#ifdef TELEMETRY_MODULES_NMEA
	#include "nmea/nmea.h"
#endif
#ifdef TELEMETRY_MODULES_MSP
	#include "msp/msp.h"
#endif

#define declare_module(NS) { telemetry::modules:: NS ::__name, telemetry::modules:: NS ::settings::init, \
	telemetry::modules:: NS ::settings::reset, telemetry::modules:: NS ::init, telemetry::modules:: NS ::update }

namespace telemetry
{

namespace modules
{

	const module_t modules [] PROGMEM = {
#ifdef TELEMETRY_MODULES_ADC_BATTERY
		declare_module (adc_battery),
#endif
#ifdef TELEMETRY_MODULES_ADC_RSSI
		declare_module (adc_rssi),
#endif
#ifdef TELEMETRY_MODULES_I2C_BARO
		declare_module (i2c_baro),
#endif
#ifdef TELEMETRY_MODULES_I2C_COMPASS
		declare_module (i2c_compass),
#endif
#ifdef TELEMETRY_MODULES_UAVTALK
		declare_module (uavtalk),
#endif
#ifdef TELEMETRY_MODULES_MAVLINK
		declare_module (mavlink),
#endif
#ifdef TELEMETRY_MODULES_UBX
		declare_module (ubx),
#endif
#ifdef TELEMETRY_MODULES_NMEA
		declare_module (nmea),
#endif
#ifdef TELEMETRY_MODULES_MSP
		declare_module (msp),
#endif
	};

	const uint8_t count = sizeof (modules) / sizeof (module_t);

}  // namespace modules


void init ()
{
	status::callsign = s.callsign;
	battery::init ();
	for (uint8_t i = 0; i < modules::count; i ++)
		modules::init (i);
}

bool update ()
{
	update_time = timer::ticks ();

	bool res = false;
	for (uint8_t i = 0; i < modules::count; i ++)
		res |= modules::update (i);
	return res;
}

bool receive (parser_t parser)
{
	uint16_t err = 0;
	do
	{
		uint16_t raw = TELEMETRY_UART::receive ();
		err = raw & 0xff00;
		if (!err && parser (raw)) return true;
	}
	while (!err);
	return false;
}

}  // namespace telemetry
