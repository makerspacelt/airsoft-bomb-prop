#!/bin/bash -e

ESPHOME_VERSION=2025.7.5

shopt -s nullglob

cd "$(dirname "$0")"

args=()
for d in /dev/ttyACM* /dev/ttyUSB*; do
    args+=("--volume=$d:$d")
done

# shellcheck disable=SC2086
exec podman run -it --rm \
    --userns keep-id \
    -e PLATFORMIO_SETTING_ENABLE_TELEMETRY=0 \
    -v "$PWD/..:/config" \
    "${args[@]}" \
    ghcr.io/esphome/esphome:$ESPHOME_VERSION \
    "$@"

