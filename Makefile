#########################################
# makefile for vatthex_indi
#########################################

VATTHEX_INDI_OBJS = vatthex.o vatt_secondary.o projectsoft.o
TEST_PROJECTSOFT_OBJS = testprojectsoft.o projectsoft.o
TESTHEX_OBJS = vatthex.o test.o

.PHONY: all clean install

all: clean indi-vatt-pihex testprojectsoft

indi-vatt-pihex: $(VATTHEX_INDI_OBJS)
	g++ -std=c++17 $^ -lindidriver -lnova -lpthread -lz -o indi-vatt-pihex -LPI -lpi_pi_gcs2 -IPI

testprojectsoft: $(TEST_PROJECTSOFT_OBJS)
	gcc $^ -o testprojectsoft

vatt_secondary.o: vatt_secondary.cpp
	g++ -std=c++17 -c vatt_secondary.cpp -I/usr/include/libindi -Ilibvatthex/include -I/usr/local/include/PI

projectsoft.o: projectsoft.c
	gcc -c projectsoft.c

testprojectsoft.o: testprojectsoft.c
	gcc -c testprojectsoft.c

vatthex.o: vatthex.c
	gcc -c -fPIC vatthex.c -IPI

test.o:
	gcc -c test.c

testhex: $(TESTHEX_OBJS)
	gcc $^ -o testhex -IPI -lpi_pi_gcs2

install:
	cp indi-vatt-pihex /usr/local/bin/.
	cp PI/libpi_pi_gcs2.so.3.9.0 /usr/local/lib/libpi_pi_gcs2.so
	cp PI/libpi_pi_gcs2-3.9.0.a /usr/local/lib/libpi_pi_gcs2.a
	ldconfig

clean:
	rm -f *.o libvatthex.so indi-vatt-pihex hextest testprojectsoft