#!/usr/bin/env bash
# Run the already-built vending_app GUI (see docker/build-ubuntu.sh) against
# the host's X11 display.
#
# --network host is required: with the default bridge network, X11's
# `xhost +local:` peer-credential check doesn't recognize the container as
# local and refuses the connection.
# --device /dev/dri passes the GPU through; without it Qt Quick still runs,
# just falling back to software rendering (Mesa prints warnings).
# Runs as root in-container: the image's `builder` user has no X11/DRI
# group membership, and there's nothing else in the container worth
# isolating root from.
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

if [ ! -x build/vending_app ]; then
  echo "build/vending_app not found — run docker/build-ubuntu.sh first" >&2
  exit 1
fi

DOCKER="docker"
if ! docker info >/dev/null 2>&1; then
  DOCKER="sudo docker"
fi

xhost +local: >/dev/null

$DOCKER run --rm -it \
  --network host \
  --user root \
  --device /dev/dri \
  -e DISPLAY="$DISPLAY" \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "$PWD":/src -w /src/build \
  vending-build:ubuntu ./vending_app
