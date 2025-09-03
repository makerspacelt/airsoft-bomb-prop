#!/bin/bash -e

cd "$(dirname "$0")"
shopt -s nullglob

# pick up the version from the github workflow
ESPHOME_VERSION=$(sed -nr 's/^ *ESPHOME_VERSION: *(.*) *$/\1/p' ../../.github/workflows/sw.yml)

args=()
for d in /dev/ttyACM* /dev/ttyUSB*; do
    args+=("--volume=$d:$d")
done

if type docker >/dev/null 2>&1; then
    container_cmd=docker
elif type podman >/dev/null 2>&1; then
    container_cmd=podman
    args+=("--userns=keep-id")
else
    echo
    echo "ERROR: docker or podman is required to run esphome"
    echo
    exit 1
fi

# shellcheck disable=SC2086
exec $container_cmd run --rm -it \
    -e PLATFORMIO_SETTING_ENABLE_TELEMETRY=0 \
    -e PLATFORMIO_BUILD_CACHE_DIR=/config/.platformio/build-cache \
    -v "$PWD/..:/config" \
    "${args[@]}" \
    ghcr.io/esphome/esphome:$ESPHOME_VERSION \
    "$@"
