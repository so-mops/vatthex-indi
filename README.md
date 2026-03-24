# VATT Hexapod INDI Driver

This repository contains the VATT secondary hexapod control software and INDI driver. It communicates with the PI hexapod controller that positions the VATT secondary mirror.

## Current deployment

The current deployment model uses a Docker container for the driver while allowing an existing host `indiserver` to launch it through a wrapper script.

The main runtime pieces are:

- `vatt_secondary.cpp/.h` — main INDI driver logic
- `vatthex.c/.h` — low-level PI hexapod helper routines and correction model
- `projectsoft.c/.h` — telescope elevation and strut temperature input
- `docker_wrapper.sh` — launches the driver container for use by the host INDI server
- `Dockerfile` — builds the container image

Legacy and retired support files have been moved to `archive/`.

## Build

Build the driver locally with:

```bash
make