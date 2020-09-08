
all: clean sdev

sdev:
	g++ -std=c++11 vatt_secondary.cpp vatttel_com.c ngclient.c -I/usr/include/libindi -Ilibvatthex/include -I/usr/local/include/PI -lindidriver -lnova -lpthread -lz -o sdev -L/usr/local/lib -lpi_pi_gcs2 -lvatthex

install: sdev
	cp sdev /usr/local/bin/VATTHEX-INDI
# START FIX
# 09-08-2020
# Dan Avner
# Commented out the lines below since docker is no longer in use
#cp systemd/VATTHEX-INDI.service /etc/systemd/system
#cp startindi.sh /usr/local/bin/startindi
#chmod a+x /usr/local/bin/startindi
#chmod 664 /etc/systemd/system/VATTHEX-INDI.service
#systemctl daemon-reload
#systemctl enable VATTHEX-INDI.service
# END FIX

clean:
	rm -f sdev


