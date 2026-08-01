#!/usr/bin/env bash
# Build the Ubuntu (native Linux) image and run configure+build+ctest
# against the current working tree via bind mount, so you don't have to
# rebuild the image on every source change.
#
# Runs as the invoking host user (--user "$(id -u):$(id -g)"), not the
# image's baked-in `builder` account: bind mounts keep the host's UID, and
# a mismatched container UID would either fail to write ./build or leave
# root-owned files behind on the host.
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

# Use sudo automatically if the current user can't talk to the Docker
# daemon directly (e.g. not in the `docker` group).
DOCKER="docker"
if ! docker info >/dev/null 2>&1; then
  DOCKER="sudo docker"
fi

$DOCKER build -f docker/ubuntu.Dockerfile -t vending-build:ubuntu --target toolchain .

$DOCKER run --rm \
  --user "$(id -u):$(id -g)" \
  -v "$PWD":/src -w /src \
  vending-build:ubuntu bash -c '
    set -euo pipefail
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENDING_BUILD_APP=ON -DVENDING_BUILD_TESTS=ON
    cmake --build build -j"$(nproc)"
    ctest --test-dir build --output-on-failure
  '
