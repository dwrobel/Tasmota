#!/bin/sh

set -e

export PLATFORMIO_BUILD_FLAGS='-DUSE_CONFIG_OVERRIDE -DSTA_PASS1=\"\" -DSTA_PASS2=\"\"'

for t in tasmota-minimal tasmota-sensors; do
    pio run -e ${t}
    cp -pf .pioenvs/${t}/firmware.bin ${t}.bin
done
