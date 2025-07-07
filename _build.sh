#!/bin/sh

set -e

export PLATFORMIO_BUILD_FLAGS='-DUSE_CONFIG_OVERRIDE -DSTA_PASS1=\"\" -DSTA_PASS2=\"\"'

# Note:
# For ESP32-D0WDQ6 v1.0 (PSRAM disabled) (https://github.com/LilyGO/TTGO-T8-ESP32.git)
# use "board = esp32-fix" in platformio_tasmota_env32.ini for [env:tasmota32].

for t in tasmota-minimal tasmota-sensors tasmota32 tasmota32solo1 tasmota32s2 tasmota32s3 tasmota32-webcam; do
    pio run -e ${t}
    cp -pf .pio/build/${t}/firmware.bin ${t}.bin
    gzip -c ${t}.bin >${t}.bin.gz

    if [ -f .pio/build/${t}/firmware.factory.bin ]; then
        cp -pf .pio/build/${t}/firmware.factory.bin ${t}.firmware.factory.bin
    fi

    if [ -d ~/public_html/tasmota/ ]; then
        REL_DIR=~/public_html/tasmota/15.0.1.1-1
        mkdir -p ${REL_DIR}
        cp -a ${t}.bin ${t}.bin.gz ${REL_DIR}/

        if [ -f ${t}.firmware.factory.bin ]; then
            cp -a ${t}.firmware.factory.bin ${REL_DIR}
        fi

        if [ -f variants/tasmota/${t}-safeboot.bin ]; then
            cp -a variants/tasmota/${t}-safeboot.bin ${REL_DIR}
        fi

        gzip -c .pio/build/${t}/firmware.elf >${REL_DIR}/${t}.elf.gz
    fi
done
