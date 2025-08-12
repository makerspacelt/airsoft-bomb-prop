#!/bin/sh -e

cd "$(dirname "$0")"

./build.sh
exec build/ant "$@"
