


all: clean sdev

sdev:
	g++ -std=c++11 vatt_secondary.cpp vatttel_com.c -I/usr/include/libindi -I/usr/local/include/PI -lindidriver -lnova -lpthread -lz -o sdev -lpi_pi_gcs2 -L/usr/local/lib/ -lvatthex

install: sdev
	mv sdev /usr/local/bin/VATTHEX-INDI

clean:
	rm -f sdev
