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
#include <TrivialKalmanFilter.h>

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

// For more information see: https://github.com/dwrobel/TrivialKalmanFilter
#ifndef KALMAN_COVARIANCE_RK
#define KALMAN_COVARIANCE_RK 4.7e-3f // Estimation of the noise covariances (process)
#endif
#ifndef KALMAN_COVARIANCE_QK
#define KALMAN_COVARIANCE_QK 1e-5f   // Estimation of the noise covariances (observation)
#endif

#define _1W_STR "%002hhX%002hhX%002hhX%002hhX%002hhX%002hhX%002hhX%002hhX"
#define _1W_ARG(a) (uint8_t) a[0], (uint8_t)a[1], (uint8_t)a[2], (uint8_t)a[3], (uint8_t)a[4], (uint8_t)a[5], (uint8_t)a[6], (uint8_t)a[7]

static OneWire ow;
static DallasTemperature dt;

struct temp_sensor {
    temp_sensor()
        : addr{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
        , errors(0)
        , temp(nan(""))
        , filter(KALMAN_COVARIANCE_RK, KALMAN_COVARIANCE_QK) {
    }

    float update(const float temp) {
        return filter.update(temp);
    }

    DeviceAddress addr;
    uint8_t errors;
    float temp;
    TrivialKalmanFilter<float> filter;
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

        if (Settings->flag5.ds18x20_mean) {
            temp = t->update(t->temp);
        }

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

        const auto temp = Settings->flag5.ds18x20_mean ? t->filter.get() : t->temp;

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

#ifdef DS18x20_USE_ID_ALIAS
const char kds18Commands[] PROGMEM = "|"  // No prefix
  D_CMND_DS_ALIAS;

void (* const DSCommand[])(void) PROGMEM = {
  &CmndDSAlias };

void CmndDSAlias(void) {
  uint8_t tmp;
  uint8_t sensor=255;
  char argument[XdrvMailbox.data_len];
  char address[17];

  if (ArgC()==2) {
    tmp=atoi(ArgV(argument, 2));
    ArgV(argument,1);

    for (uint32_t i = 0; i < DS18X20Data.sensors; i++) {
      for (uint32_t j = 0; j < 8; j++) {
        sprintf(address+2*j, "%02X", ds18x20_sensor[i].address[7-j]);
      }
      if (!strncmp(argument,address,12)) {
        ds18x20_sensor[i].alias=tmp;
        break;
      }
    }
  }
  Response_P(PSTR("{"));
  for (uint32_t i = 0; i < DS18X20Data.sensors; i++) {
    Ds18x20Name(i);
    char address[17];
    for (uint32_t j = 0; j < 8; j++) {
      sprintf(address+2*j, "%02X", ds18x20_sensor[i].address[7-j]);  // Skip sensor type and crc
    }
    ResponseAppend_P(PSTR("\"%s\":{\"" D_JSON_ID "\":\"%s\"}"),DS18X20Data.name, address);
    if (i < DS18X20Data.sensors-1) {ResponseAppend_P(PSTR(","));}
  }
  ResponseAppend_P(PSTR("}"));
}
#endif // DS18x20_USE_ID_ALIAS

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns05(const uint32_t function) {
    if (!(PinUsed(GPIO_DSB) || PinUsed(GPIO_DSB_OUT))) {
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
