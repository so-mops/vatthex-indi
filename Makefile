
all: clean sdev

sdev:
	g++ -std=c++11 vatt_secondary.cpp vatttel_com.c ngclient.c -I/usr/include/libindi -Ilibvatthex/include -I/usr/local/include/PI -lindidriver -lnova -lpthread -lz -o sdev -L/usr/local/lib -lpi_pi_gcs2 -lvatthex

install: sdev
	cp sdev /usr/local/bin/VATTHEX-INDI
	cp systemd/VATTHEX-INDI.service /etc/systemd/system
	cp startindi.sh /usr/local/bin/startindi
	chmod a+x /usr/local/bin/startindi
	chmod 664 /etc/systemd/system/VATTHEX-INDI.service
	systemctl daemon-reload
	systemctl enable VATTHEX-INDI.service

clean:
	rm -f sdev


