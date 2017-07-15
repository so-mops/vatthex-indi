


all: clean sdev

sdev:
	g++ -std=c++11 scottdevice.cpp -I/usr/include/libindi -I/usr/local/include/PI -lindidriver -lnova -lpthread -lz -o sdev -lpi_pi_gcs2 -L/usr/local/lib/ -lvatthex

clean:
	rm -f sdev
