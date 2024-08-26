#!/bin/sh

set -e

export PLATFORMIO_BUILD_FLAGS='-DUSE_CONFIG_OVERRIDE -DSTA_PASS1=\"\" -DSTA_PASS2=\"\"'

# Note:
# For ESP32-D0WDQ6 v1.0 (PSRAM disabled) (https://github.com/LilyGO/TTGO-T8-ESP32.git)
# use "board = esp32-fix" in platformio_tasmota_env32.ini for [env:tasmota32].

for t in tasmota-minimal tasmota-sensors tasmota32 tasmota32solo1 tasmota32s2; do
    pio run -e ${t}
    cp -pf .pio/build/${t}/firmware.bin ${t}.bin
    gzip -c ${t}.bin >${t}.bin.gz

    if [ -f .pio/build/${t}/firmware.factory.bin ]; then
        cp -pf .pio/build/${t}/firmware.factory.bin ${t}.firmware.factory.bin
    fi

    if [ -d ~/public_html/tasmota/ ]; then
        cp -a ${t}.bin ${t}.bin.gz ~/public_html/tasmota/

        if [ -f ${t}.firmware.factory.bin ]; then
            cp -a ${t}.firmware.factory.bin ~/public_html/tasmota/
        fi
    fi
done
