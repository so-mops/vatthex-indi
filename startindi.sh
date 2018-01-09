#!/bin/bash


datestamp=`date +%H%M%M%m%Y`
mkdir -p /home/scott/.indilogs/${datestamp}
/usr/bin/indiserver -vv -l /home/scott/.indilogs/${datestamp} /usr/local/bin/VATTHEX-INDI
