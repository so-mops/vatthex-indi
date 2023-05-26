#########################################
# makefile for vatthex_indi	  	#
################objects##################
OLD_OBJS = vatt_secondary.o vatttel_com.o ngclient.o
VATTHEX-INDI-OBJS = vatthex.o vatt_secondary.o vatttel_com.o ngclient.o readcbw.o mjson.o
LIBVATTHEX_OBJS = vatthex.o
###############binaries##################
all: clean hextest indi-vatt-pihex testcbw

all-old: libvatthex.so VATTHEX-INDI-old 

VATTHEX-INDI-old:  $(OLD_OBJS)
	g++ -std=c++11 $^ -lindidriver -lnova -lpthread -lz -o VATTHEX-INDI-old -L/usr/local/lib -lpi_pi_gcs2 -lvatthex

indi-vatt-pihex:  $(VATTHEX-INDI-OBJS)
	g++ -std=c++11 $^ -lindidriver -lnova -lpthread -lz -o indi-vatt-pihex -L/usr/local/lib -lpi_pi_gcs2 -lcurl 

testcbw: mjson.o readcbw.o testcbw.o vatttel_com.o
	gcc $^ -lcurl -o testcbw

vatt_secondary.o: vatt_secondary.cpp
	g++ -std=c++11 -c vatt_secondary.cpp -I/usr/include/libindi -Ilibvatthex/include -I/usr/local/include/PI

vatttel_com.o: vatttel_com.c
	gcc -c vatttel_com.c
#g++ -std=c++11 -c vatttel_com.c

ngclient.o: ngclient.c
	gcc -c ngclient.c 
#g++ -c ngclient.c

vatthex.o: vatthex.c
	gcc -c -fPIC vatthex.c -IPI

mjson.o: mjson.c mjson.h
	gcc -c mjson.c
        
readcbw.o: readcbw.c
	gcc -c readcbw.c

testcbw.o: testcbw.c
	gcc -c testcbw.c

libvatthex.so: $(LIBVATTHEX_OBJS)
	gcc -shared $^ -IPI -o libvatthex.so

hextest:
	gcc test.c -o hextest -I./PI/include -Llib -lvatthex -lpi_pi_gcs2

###############Utilities################
install: 
	cp indi-vatt-pihex /usr/local/bin/.
	cp PI/libpi_pi_gcs2.so.3.9.0 /usr/local/lib/libpi_pi_gcs2.so
	cp PI/libpi_pi_gcs2-3.9.0.a /usr/local/lib/libpi_pi_gcs2.a
	#cp libvatthex.so /usr/local/lib/.
	#cp vatthex.h /usr/local/include/.
	ldconfig
 
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
	rm -f *.o libvatthex.so indi-vatt-pihex hextest testcbw VATTHEX-INDI-old


