FROM ubuntu:22.04 AS build
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    make \
    gcc \
    g++ \
    ca-certificates \
    gnupg \
    software-properties-common \
 && rm -rf /var/lib/apt/lists/*

RUN add-apt-repository -y ppa:mutlaqja/ppa

RUN apt-get update && apt-get install -y --no-install-recommends \
    libindi-dev \
    libnova-dev \
    zlib1g-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY . /build

RUN make

FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    gnupg \
    software-properties-common \
 && rm -rf /var/lib/apt/lists/*

RUN add-apt-repository -y ppa:mutlaqja/ppa

RUN apt-get update && apt-get install -y --no-install-recommends \
    libindi1 \
    libnova-0.16-0 \
    zlib1g \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/vatthex

COPY --from=build /build/indi-vatt-pihex /opt/vatthex/indi-vatt-pihex
COPY PI/libpi_pi_gcs2.so* /usr/local/lib/

RUN mkdir -p /var/lib/vatthex \
 && chmod +x /opt/vatthex/indi-vatt-pihex \
 && ldconfig

ENV VATTHEX_DATA_DIR=/var/lib/vatthex

VOLUME ["/var/lib/vatthex"]

ENTRYPOINT ["/opt/vatthex/indi-vatt-pihex"]