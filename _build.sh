#!/bin/sh

set -e

export PLATFORMIO_BUILD_FLAGS='-DUSE_CONFIG_OVERRIDE -DSTA_PASS1=\"\" -DSTA_PASS2=\"\"'

for t in tasmota-minimal tasmota-sensors tasmota32 tasmota32s2; do
    pio run -e ${t}
    cp -pf .pio/build/${t}/firmware.bin ${t}.bin
    gzip -c ${t}.bin >${t}.bin.gz
    cp -pf .pio/build/${t}/firmware.factory.bin ${t}.firmware.factory.bin || true

    if [ -d ~/public_html/tasmota/ ]; then
        cp -a ${t}.bin ${t}.bin.gz ~/public_html/tasmota/
        cp -a ${t}.firmware.factory.bin ~/public_html/tasmota/ || true
    fi
done
