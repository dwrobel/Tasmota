/*
  xsns_05_ds18x20.ino - DS18x20 temperature sensor support for Tasmota

  Copyright (C) 2022  Damian Wrobel <dwrobel@ertelnet.rybnik.pl>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_DS18x20
#include <DallasTemperature.h>

/*********************************************************************************************\
 * DS18B20 - Temperature - Multiple sensors
\*********************************************************************************************/

#define XSNS_05              5

#ifndef DS18X20_MAX_SENSORS // DS18X20_MAX_SENSORS fallback to 8 if not defined in user_config_override.h
#   define DS18X20_MAX_SENSORS 8
#endif

#ifndef _1WIRE_MAX_ERRORS
#   define _1WIRE_MAX_ERRORS 3
#endif

#define _1W_STR "%002hhX%002hhX%002hhX%002hhX%002hhX%002hhX%002hhX%002hhX"
#define _1W_ARG(a) (uint8_t) a[0], (uint8_t)a[1], (uint8_t)a[2], (uint8_t)a[3], (uint8_t)a[4], (uint8_t)a[5], (uint8_t)a[6], (uint8_t)a[7]

static OneWire ow;
static DallasTemperature dt;

struct temp_sensor {
    temp_sensor()
        : addr{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
        , errors(0)
        , temp(nan("")) {
    }

    DeviceAddress addr;
    uint8_t errors;
    float temp;
} sensors[DS18X20_MAX_SENSORS];


static int8_t addr2idx(const uint8_t *const addr) {
    for (uint8_t i = 0; i < DS18X20_MAX_SENSORS; i++) {
        if (memcmp(sensors[i].addr, addr, sizeof(DeviceAddress)) == 0) {
            return i;
        }
    }

    return -1;
}


static bool is_free_idx(const int8_t idx) {
    if (idx < 0 || idx > (DS18X20_MAX_SENSORS - 1)) {
        return false;
    }

    return !dt.validFamily(sensors[idx].addr) ? true : false;
}


static int8_t get_first_free_idx() {
    for (uint8_t i = 0; i < DS18X20_MAX_SENSORS; i++) {
        if (is_free_idx(i))
        return i;
    }

    return -1;
}


static const char model_name[] PROGMEM = "Unknown|DS18x20|DS18B20|DS1822|DS18B25|DS28EA00";
static const uint8_t model_name_ids[]  = { 0xff, DS18S20MODEL, DS18B20MODEL, DS1822MODEL, DS1825MODEL, DS28EA00MODEL };


static int8_t get_model_name_index(const uint8_t ids) {
    for (uint8_t i = 0; i < sizeof(model_name_ids) / sizeof(model_name_ids[0]); i++) {
        if (model_name_ids[i] == ids)
            return i;
    }

    return 0;
}


static const char *sensor_name(const int8_t index) {
    static char name[16] = {0,};
    static int8_t last_index = -1;

    if (index != last_index || index < 0) {
        if (is_free_idx(index)) {
            snprintf(name, sizeof(name), "%s-%hhd", "Unknown", index + 1);
        } else {
            GetTextIndexed(name, sizeof(name), get_model_name_index(sensors[index].addr[0]), model_name);
            const uint8_t name_len = strlen(name);
            snprintf(name + name_len, sizeof(name) - name_len, "-%hhd", index + 1);
        }

        last_index = index;
    }

    return name;
}


static void report_errors(struct temp_sensor *const t, const uint8_t error, const bool force_report_state = false) {
    bool report_state = false;

    do {
        // success
        if (!error) {
            t->errors = 0;
            break;
        }

        // communication error
        t->errors++;

        if (t->errors > _1WIRE_MAX_ERRORS) {
            t->errors = _1WIRE_MAX_ERRORS;
        }

        if (t->errors == _1WIRE_MAX_ERRORS) { // report error
            report_state = true;
            break;
        }
    } while (0);

    if (report_state || force_report_state) {
//      DEBUG_OUTPUT(PSTR("%S%hhu e:%hhu a:" _1W_STR " F:%d->%d\n"), description, t->sensor_id, t->errors, _1W_ARG(t->addr), !t->errors, !!t->errors);
    }

};


static void Ds18x20Init(void) {
    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_DSB "Ds18x20Init: max. devices: %d"), DS18X20_MAX_SENSORS);

    ow.begin(Pin(GPIO_DSB));
    dt.setOneWire(&ow);
    dt.begin();

    const uint8_t num_dev = dt.getDeviceCount();

    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_DSB "Ds18x20Init: devices: %hhu, max. supported devices: %d"), num_dev, DS18X20_MAX_SENSORS);

    if (num_dev == 0) { // only for deubugging purpose
        ow.reset_search();

        DeviceAddress addr;

        while (ow.search(addr)) {
            const uint8_t crc = ow.crc8(addr, 7);

            if (crc != addr[7]) { // can detect some type of connection problems
                AddLog(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_DSB "Ds18x20Init: a:" _1W_STR " crc: %02hhx != %02hhx"), _1W_ARG(addr), crc, addr[7]);
            }
        }

        return;
    }

    for (uint8_t i = 0; i < num_dev; i++) {
        DeviceAddress da;

        if (!dt.getAddress(da, i)) {
            AddLog(LOG_LEVEL_ERROR, PSTR(D_LOG_DSB "Ds18x20Init[%hhu]: could not retrieve device address"), i);
            continue;
        }

        if (!dt.validFamily(da)) {
            AddLog(LOG_LEVEL_DEBUG, PSTR(D_LOG_DSB "Ds18x20Init[%hhu]: id: " _1W_STR " unsupported device type"), i, _1W_ARG(da));
            continue;
        }

        if (addr2idx(da) >= 0) {
            AddLog(LOG_LEVEL_ERROR, PSTR(D_LOG_DSB "Ds18x20Init[%hhu]: id: " _1W_STR " duplicate device connected to the bus"), i, _1W_ARG(da));
            continue;
        }

        const int8_t idx = get_first_free_idx();

        if (idx < 0) {
            AddLog(LOG_LEVEL_ERROR, PSTR(D_LOG_DSB "Ds18x20Init[%hhu]: id: " _1W_STR " too many devices connected to the bus"), i, _1W_ARG(da));
            continue;
        }

        temp_sensor *const t = &sensors[idx];
        memcpy(&t->addr, da, sizeof(DeviceAddress));

        AddLog(LOG_LEVEL_DEBUG, PSTR(D_LOG_DSB "Ds18x20Init[%hhu]: type: %s, id: " _1W_STR), idx, sensor_name(idx), _1W_ARG(t->addr));
    }

    dt.setWaitForConversion(false);
    dt.requestTemperatures();
}

static void Ds18x20EverySecond(void) {
    if (!dt.isConversionComplete()) {
        AddLog(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_DSB "Ds18x20EverySecond: conversion is in progress..."));
        return;
    }

    for (uint8_t i = 0; i < DS18X20_MAX_SENSORS; i++) {
        temp_sensor *const t = &sensors[i];

        if (!dt.validFamily(t->addr)) {
            break;
        }

        const int16_t temp_raw = dt.getTemp(t->addr);

        if (temp_raw == DEVICE_DISCONNECTED_RAW) {
            report_errors(t, +1);
            AddLog(LOG_LEVEL_ERROR, PSTR(D_LOG_DSB "Ds18x20EverySecond[%hhu]: id:" _1W_STR " errors: %hhu"), i, _1W_ARG(t->addr), t->errors);
            continue;
        }

        const float temp_C = dt.rawToCelsius(temp_raw);

        auto temp = t->temp = ConvertTemp(temp_C);

        AddLog(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_DSB "Ds18x20EverySecond[%hhu]: id:" _1W_STR " errors: %hhu temp: %*_f"),
               i,
               _1W_ARG(t->addr),
               t->errors,
               Settings->flag2.temperature_resolution,
               &temp);

        report_errors(t, 0);
    }

    dt.requestTemperatures();
}

static void Ds18x20Show(bool json) {
    for (uint8_t i = 0; i < DS18X20_MAX_SENSORS; i++) {
        temp_sensor *const t = &sensors[i];

        if (!dt.validFamily(t->addr)) {
            break;
        }

        if (t->errors != 0) { // Check for valid temperature
            continue;
        }

        const auto temp = t->temp;

        if (json) {
            ResponseAppend_P(PSTR(",\"%s\":{\"" D_JSON_ID "\":\"" _1W_STR "\",\"" D_JSON_TEMPERATURE "\":%*_f}"),
                             sensor_name(i),
                             _1W_ARG(t->addr),
                             Settings->flag2.temperature_resolution,
                             &temp);

#           ifdef USE_DOMOTICZ
            if ((0 == TasmotaGlobal.tele_period) && (0 == i)) {
                DomoticzSensor(DZ_TEMP, temp);
            }
#           endif // USE_DOMOTICZ

#           ifdef USE_KNX
            if ((0 == TasmotaGlobal.tele_period) && (0 == i)) {
                KnxSensor(KNX_TEMPERATURE, temp);
            }
#           endif // USE_KNX
#       ifdef USE_WEBSERVER
        } else {
            WSContentSend_Temp(sensor_name(i), temp);
#       endif  // USE_WEBSERVER
        }
    }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns05(const uint8_t function) {
    if (!PinUsed(GPIO_DSB)) {
        return false;
    }

    switch (function) {
    case FUNC_INIT:
        Ds18x20Init();
    break;
    case FUNC_EVERY_SECOND:
        Ds18x20EverySecond();
    break;
    case FUNC_JSON_APPEND:
        Ds18x20Show(true);
    break;
#   ifdef USE_WEBSERVER
    case FUNC_WEB_SENSOR:
        Ds18x20Show(false);
    break;
#   endif  // USE_WEBSERVER
    }

    return false;
}

#endif  // USE_DS18x20
