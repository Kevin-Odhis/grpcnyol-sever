# =========================
# Build Stage
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Base dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    libssl-dev \
    protobuf-compiler \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

# -------------------------
# Install vcpkg
WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg.git
WORKDIR /opt/vcpkg
RUN ./bootstrap-vcpkg.sh

# -------------------------
# Install dependencies via vcpkg
RUN ./vcpkg install \
    grpc \
    protobuf \
    mongo-cxx-driver

# -------------------------
# App build
WORKDIR /app
COPY . .

# Tell CMake to use vcpkg toolchain
RUN mkdir build
WORKDIR /app/build

RUN cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

#RUN make -j$(nproc)
RUN make -j2

# =========================
# Runtime Stage
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/nyolapp .

EXPOSE 50051

CMD ["./nyolapp"]