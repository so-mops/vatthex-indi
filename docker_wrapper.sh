#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_DIR="${DATA_DIR:-$SCRIPT_DIR/vatthex-data}"

mkdir -p "$DATA_DIR"

exec docker run --rm -i \
  --name indi-vatt-pihex \
  -e VATTHEX_DATA_DIR=/var/lib/vatthex \
  -v "$DATA_DIR:/var/lib/vatthex" \
  vatthex:latest