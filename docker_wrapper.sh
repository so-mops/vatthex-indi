#!/usr/bin/env bash
set -euo pipefail

DATA_DIR="/home/vattobs/.mtnops"

mkdir -p "$DATA_DIR"

exec docker run --rm -i \
  --name indi-vatt-pihex \
  -e VATTHEX_DATA_DIR=/var/lib/vatthex \
  -v "$DATA_DIR:/var/lib/vatthex" \
  vatthex:latest