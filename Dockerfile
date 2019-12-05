FROM ubuntu:bionic

RUN apt-get update

RUN apt-get -y install \
	cmake g++ libindi-dev indi-bin libnova-dev zlib1g-dev
#        cdbs dpkg-dev debhelper cmake curl dcraw fakeroot wget git ssh \
#        libcurl4-gnutls-dev libboost-dev libboost-regex-dev libcfitsio-dev \
#        libftdi-dev libdc1394-22-dev libgphoto2-dev libgps-dev libgsl-dev libjpeg-dev libtiff-dev \
#        libnova-dev libopenal-dev libraw-dev libusb-1.0-0-dev librtlsdr-dev \
#        libfftw3-dev zlib1g-dev libconfuse-dev python3-all-dev doxygen \
#        libboost-test-dev python-all-dev swig g++ libftdi1-dev \
#        libdc1394-22-dev googletest clang-5.0 lsb-release dirmngr vim \
#        libavcodec-dev libavdevice-dev libavformat-dev libavutil-dev libindi-dev indi-bin


COPY . /driver/build


RUN cd /driver/build/libvatthex && make install
RUN cd /driver/build && make 



EXPOSE 7624

#CMD sleep 1000
CMD /usr/bin/indiserver -vvv /driver/build/sdev /driver/build/indi-vatt-guidebox



