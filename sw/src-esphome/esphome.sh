#!/bin/bash -e

ESPHOME_VERSION=2025.7.5

shopt -s nullglob

cd "$(dirname "$0")"

args=()
for d in /dev/ttyACM* /dev/ttyUSB*; do
    args+=("--volume=$d:$d")
done

if type docker >/dev/null 2>&1; then
    container_cmd=docker
    container_args=
elif type podman >/dev/null 2>&1; then
    container_cmd=podman
    container_args="--userns keep-id"
else
    echo
    echo "ERROR: docker or podman is required to run esphome"
    echo
    exit 1
fi

# shellcheck disable=SC2086
exec $container_cmd run $container_args -it --rm \
    -e PLATFORMIO_SETTING_ENABLE_TELEMETRY=0 \
    -v "$PWD/..:/config" \
    "${args[@]}" \
    ghcr.io/esphome/esphome:$ESPHOME_VERSION \
    "$@"

