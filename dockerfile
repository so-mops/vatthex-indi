FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates gnupg software-properties-common \
 && rm -rf /var/lib/apt/lists/*

RUN add-apt-repository -y ppa:mutlaqja/ppa

RUN apt-get update && apt-get install -y --no-install-recommends \
    indi-bin \
    libindi1 \
    libnova-0.16-0 \
    zlib1g \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/vatthex
COPY indi-vatt-pihex /opt/vatthex/indi-vatt-pihex
COPY PI/libpi_pi_gcs2.so* /usr/local/lib/
RUN chmod +x /opt/vatthex/indi-vatt-pihex && ldconfig

EXPOSE 7600
CMD ["bash","-lc","indiserver -v -p 7624 /opt/vatthex/indi-vatt-pihex"]