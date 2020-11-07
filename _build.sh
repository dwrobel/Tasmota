#!/bin/sh

set -e

export PLATFORMIO_BUILD_FLAGS='-DUSE_CONFIG_OVERRIDE -DSTA_PASS1=\"\" -DSTA_PASS2=\"\"'
#pio lib install mbed-sam-grove/LinkedList

for t in tasmota-minimal tasmota-sensors; do
    pio run -e ${t}
    cp -pf .pio/build/${t}/firmware.bin ${t}.bin
    gzip -f ${t}.bin
done
