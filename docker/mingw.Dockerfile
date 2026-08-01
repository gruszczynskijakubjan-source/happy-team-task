# Cross-compile image: Linux host, produces Windows .exe via mingw-w64.
#
# This is NOT a native Windows container (Docker can't run those on a Linux
# host) — it's a Linux image with a MinGW-w64 cross-toolchain and MXE
# (https://mxe.cc) providing Qt6 + SQLite3 built for x86_64-w64-mingw32.
# Output binaries run on Windows (tested via Wine below, ship as .exe).
#
# Building MXE's Qt6 from source is heavy (expect 45-90+ min, several GB)
# because no prebuilt MXE Qt6 packages are distributed — this is the
# tradeoff for a build that doesn't depend on any external Windows host.
#
# Usage: see docker/build-mingw.sh, or manually:
#   docker build -f docker/mingw.Dockerfile -t vending-build:mingw .
#   docker run --rm -v "$PWD":/src -w /src vending-build:mingw bash -c \
#       "x86_64-w64-mingw32.shared-cmake -B build-mingw -G Ninja -DVENDING_BUILD_APP=ON && \
#        cmake --build build-mingw -j"
#
# Or just run the container with no override to build the copy baked into
# the image (see CMD at the bottom). Core/adapters (no Qt) cross-compile in
# minutes; the full Qt app is what makes this image slow to build.

FROM debian:bookworm AS mxe-builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        autoconf automake autopoint bash bison bzip2 flex gettext \
        git g++ gperf intltool libc6-dev-i386 libgdk-pixbuf2.0-dev \
        libltdl-dev libssl-dev libtool-bin libxml-parser-perl make \
        openssl p7zip-full patch perl pkg-config python3 python3-mako \
        python3-packaging ruby sed unzip wget xz-utils g++-multilib \
        libtool-bin lzip ca-certificates cmake ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt
RUN git clone --depth 1 https://github.com/mxe/mxe.git

WORKDIR /opt/mxe
# qtbase + qtdeclarative + qtquickcontrols2 pull in the Qt modules this
# project needs (Core/Gui/Qml/Quick/QuickControls2/Network); sqlite ships
# separately for adapters/sqlite. shared linking so runtime DLLs are
# distinct and swappable; static is available by switching MXE_TARGET.
RUN make MXE_TARGETS='x86_64-w64-mingw32.shared' \
        sqlite qtbase qtdeclarative qtquickcontrols2 \
        JOBS=$(nproc) -j$(nproc)

FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        cmake ninja-build git ca-certificates wine64 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=mxe-builder /opt/mxe /opt/mxe
ENV PATH="/opt/mxe/usr/bin:${PATH}"
ENV MXE_TOOLCHAIN=/opt/mxe/usr/x86_64-w64-mingw32.shared/share/cmake/mxe-conf.cmake

WORKDIR /src
COPY . /src

# build-mingw/ (not build/) so this never collides with a native Linux
# build directory when the repo is bind-mounted for both images.
# x86_64-w64-mingw32.shared-cmake wraps cmake with MXE's toolchain file.
RUN x86_64-w64-mingw32.shared-cmake -B build-mingw -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DVENDING_BUILD_APP=ON \
        -DVENDING_BUILD_TESTS=ON \
    && cmake --build build-mingw -j"$(nproc)"

# Smoke-test the cross-compiled test binary under Wine so a broken build
# fails at `docker build` time, not on someone's Windows machine.
CMD ["wine64", "build-mingw/tests/vending_core_tests.exe"]
