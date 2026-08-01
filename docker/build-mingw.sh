#!/usr/bin/env bash
# Build the MinGW cross-compile image and produce Windows .exe binaries in
# ./build (via bind mount) using the MXE toolchain baked into the image.
#
# First run is slow (MXE builds Qt6 from source, tens of minutes to ~1.5h
# depending on hardware); subsequent runs reuse the cached Docker layer as
# long as docker/mingw.Dockerfile and MXE's pinned revision don't change.
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

docker build -f docker/mingw.Dockerfile -t vending-build:mingw .

docker run --rm -v "$PWD":/src -w /src vending-build:mingw bash -c '
  set -euo pipefail
  x86_64-w64-mingw32.shared-cmake -B build-mingw -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DVENDING_BUILD_APP=ON -DVENDING_BUILD_TESTS=ON
  cmake --build build-mingw -j"$(nproc)"
  wine64 build-mingw/tests/vending_core_tests.exe
'
