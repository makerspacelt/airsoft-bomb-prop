#!/bin/sh -e

cd "$(dirname "$0")"

args=

if [ -c /dev/ttyACM0 ]; then
    args="$args -v /dev/ttyACM0:/dev/ttyACM0"
fi

# shellcheck disable=SC2086
exec podman run -it --rm --userns keep-id $args -v "$PWD:/config" ghcr.io/esphome/esphome "$@"
