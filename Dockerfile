# =========================
# Build Stage
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    wget \
    curl \
    protobuf-compiler \
    libprotobuf-dev \
    libgrpc++-dev \
    grpc-proto \
    libssl-dev \
    libsasl2-dev \
    && rm -rf /var/lib/apt/lists/*

# Install MongoDB C++ Driver
RUN apt-get update && apt-get install -y \
    libmongocxx-dev \
    libbsoncxx-dev

WORKDIR /app

COPY . .

RUN mkdir -p build

WORKDIR /app/build

RUN cmake ..

RUN make -j$(nproc)

# Runtime Stage
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libprotobuf-dev \
    libgrpc++1.51 \
    libmongocxx-dev \
    libbsoncxx-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/nyolapp .

EXPOSE 50051

CMD ["./nyolapp"]