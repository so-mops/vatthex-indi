# VATT Hexapod Control Software with INDI driver

This is the repository for the VATT hexapod source code. It runs the PI hexapod that positions the VATT secondary mirror.

## Undockerized
This does not use docker anymore. Simply make the build and the indiserver launcher on desktop runs it on port 7600

*Back to the README.*
_______

## Build

Simply use the Makefile with 

```
make
```
This will build the ```indi-vatt-pihex``` binary. 

## Current implementation at VATT
This software is an INDI driver and is meant to be run with an indiserver. Typically that looks something like:

```
indiserver ./indi-vatt-pihex
```

## Files
vatttel_com.c/.h:  This is the communication with vatttel using the old mixed ascii/binary vatt protocol.  we use this only to get the temperature

ngclient.c/.h: this is the communication with vatttel using the newer ng protocol.  We use this to get the elevation

vatthex.c/.h: this is the communication witht pi hexapod.  Its basically a wrapper around the pi library

vatt_secondary.cpp/.h: this is the meat of the software.  This has all the algorithms and the indi driver built in. 

test.c: This is a set of command line tools to test the PI hexapod.


##Old Docker Info [unused, kept for historical reasons]
Because of the portability issues with INDI, it is currently run in a [docker container](https://hub.docker.com/r/srswinde/indihex). This container can be built with [build.sh](https://github.com/so-mops/vatthex-indi/blob/master/build.sh). Building the dockerfile will also make the indidriver and build all the 
libvatthex stuff. See [the Dockerfile](https://github.com/so-mops/vatthex-indi/blob/master/Dockerfile) for details. 

It could be run standalone by simply running the indiserver command as shown above.

The docker container is run in docker-compose fashion so the INDI driver is bundled with the INDI webclient and the webserver. As it stands now both this INDI driver and the [vatt-guidebox](https://github.com/so-mops/vatt-guidebox) INDI drivers are run in the same docker-compose bundle. The docker-compose file can be found [here](https://github.com/srswinde/indi_webclient/blob/master/docker-compose-vatt-guidebox.yml). 

To have the docker-compose file run on startup we build a systemd service. This process is described in the [indi_webclient readme](https://github.com/srswinde/indi_webclient/blob/master/README.md). 
