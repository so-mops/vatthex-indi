# VATT Hexapod Control Software with INDI driver

This is the repository for the VATT hexapod source code. It runs the PI hexapod that positions the VATT secondary mirror.


## Build

Simply use the Makefile with 

```
make
```
This will build the ```sdev``` binary. 

## Current implementation at VATT
This software is an INDI driver and is meant to be run with an indiserver. Typically that looks something like:

```
indiserver ./sdev
```

Because of the portability issues with INDI, it is currently run in a [docker container](https://hub.docker.com/r/srswinde/indihex). This container can be built with build.sh. Building the dockerfile will also make the indidriver and build all the 
libvatthex stuff. See [the Dockerfile](https://github.com/so-mops/vatthex-indi/blob/master/Dockerfile) for details. 

It could be run standalone by simply running the indiserver command as shown above.

The docker container is run in docker-compose fashion so the INDI driver is bundled with the INDI webclient and the webserver. As it stands now both this INDI driver and the [vatt-guidebox](https://github.com/so-mops/vatt-guidebox) INDI drivers are run in the same docker-compose bundle. The docker-compose file can be found [here](https://github.com/srswinde/indi_webclient/blob/master/docker-compose-vatt-guidebox.yml). 

To have the docker-compose file run on startup we build a systemd service. This process is described in the [indi_webclient readme](https://github.com/srswinde/indi_webclient/blob/master/README.md). 
