# Native Linux build/test image (Ubuntu 22.04).
#
# Two stages:
#   toolchain - just the OS + build tools + Qt6/SQLite/GTest, no sources.
#               Build this once, then bind-mount the repo into it for fast
#               iterative local builds (see docker/build-ubuntu.sh).
#   build     - toolchain + COPY of the current tree, built at image-build
#               time so `docker build` fails fast on a broken tree (e.g. CI
#               without a bind mount).
#
# Usage (toolchain, bind-mounted — fast iteration):
#   docker build -f docker/ubuntu.Dockerfile --target toolchain -t vending-build:ubuntu .
#   docker run --rm -v "$PWD":/src -w /src vending-build:ubuntu bash -c \
#       "cmake -B build -G Ninja -DVENDING_BUILD_APP=ON && cmake --build build -j && ctest --test-dir build"
#
# Usage (build, self-contained — CI):
#   docker build -f docker/ubuntu.Dockerfile -t vending-build:ubuntu-ci .
#   docker run --rm vending-build:ubuntu-ci   # runs ctest by default

FROM ubuntu:22.04 AS toolchain

ENV DEBIAN_FRONTEND=noninteractive \
    TZ=UTC

# Toolchain + full Qt6/QML stack (ui_service) + Boost (cloud_service's
# Beast/Asio-based RestTransport) + SQLite dev headers + GTest.
# qt6-base/qt6-declarative give Core/Gui/Qml/Quick/QuickControls2;
# libsqlite3-dev satisfies database_service's find_package(SQLite3).
# libgl1-mesa-dev/libglu1-mesa-dev/libqt6opengl6-dev: qt6-base-dev does not
# pull these in on 22.04, but Qt6Gui's WrapOpenGL check and Qt6Quick's
# Qt6OpenGL dependency both require them — without all three,
# Qt6Gui/Qt6Quick/Qt6QuickControls2 fail to configure even though
# qt6-base-dev itself installs cleanly.
# libboost-system-dev pulls in Boost's CMake config files + the
# boost::system compiled component; Beast/Asio themselves are header-only.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        git \
        pkg-config \
        ca-certificates \
        libsqlite3-dev \
        libboost-system-dev \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        libqt6opengl6-dev \
        qt6-base-dev \
        qt6-declarative-dev \
        qml6-module-qtquick \
        qml6-module-qtquick-controls \
        qml6-module-qtquick-layouts \
        qml6-module-qtquick-templates \
        qml6-module-qtquick-window \
        qml6-module-qtqml-workerscript \
        libgtest-dev \
        libgmock-dev \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --create-home --shell /bin/bash builder \
    && mkdir -p /src && chown builder:builder /src

WORKDIR /src
USER builder

FROM toolchain AS build

COPY --chown=builder:builder . /src

RUN cmake -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DVENDING_BUILD_APP=ON \
        -DVENDING_BUILD_TESTS=ON \
    && cmake --build build -j"$(nproc)"

CMD ["ctest", "--test-dir", "build", "--output-on-failure"]
