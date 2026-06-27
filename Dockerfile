
FROM debian:bookworm-slim AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_FORCE_SYSTEM_BINARIES=1

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ca-certificates \
    libssl-dev \
    protobuf-compiler \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git

WORKDIR /opt/vcpkg

RUN ./bootstrap-vcpkg.sh

RUN ./vcpkg install \
    grpc \
    protobuf \
    mongo-cxx-driver \
    --triplet x64-linux

WORKDIR /app

COPY . .

RUN cmake -B build -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_CXX_FLAGS="-O2 -pipe -DNDEBUG"

RUN cmake --build build -j1

#Runtime

FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib

RUN apt-get update && apt-get install -y \
    libssl3 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/nyolapp .

COPY --from=builder /opt/vcpkg/installed/x64-linux/lib /usr/local/lib

RUN ldconfig

EXPOSE 50051

CMD ["./nyolapp"]