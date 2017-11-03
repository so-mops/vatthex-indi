


all: clean sdev

sdev:
	g++ -std=c++11 vatt_secondary.cpp vatttel_com.c -I/usr/include/libindi -I/usr/local/include/PI -lindidriver -lnova -lpthread -lz -o sdev -lpi_pi_gcs2 -L/usr/local/lib/ -lvatthex

install: sdev
	cp sdev /usr/local/bin/VATTHEX-INDI
	cp systemd/VATTHEX-INDI.service /etc/systemd/system
	chmod 664 /etc/systemd/system/VATTHEX-INDI.service
	systemctl daemon-reload
	systemctl enable VATTHEX-INDI.service

clean:
	rm -f sdev


