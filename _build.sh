#!/bin/sh

set -e

for t in tasmota-minimal tasmota-sensors; do
    pio run -e ${t}
    cp -pf .pioenvs/${t}/firmware.bin ${t}.bin
done
